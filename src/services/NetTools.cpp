#include "services/NetTools.h"
#include "ui/Theme.h"

#include "ping/ping_sock.h"
#include "lwip/ip_addr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace nettools {

struct NamedPort { uint16_t port; const char* name; };
static const NamedPort PORT_TABLE[] = {
    {21,"ftp"},{22,"ssh"},{23,"telnet"},{25,"smtp"},{53,"dns"},{80,"http"},
    {110,"pop3"},{111,"rpc"},{135,"msrpc"},{139,"netbios"},{143,"imap"},
    {161,"snmp"},{389,"ldap"},{443,"https"},{445,"smb"},{465,"smtps"},
    {587,"submission"},{631,"ipp"},{993,"imaps"},{995,"pop3s"},
    {1433,"mssql"},{1723,"pptp"},{3306,"mysql"},{3389,"rdp"},{5060,"sip"},
    {5432,"postgres"},{5900,"vnc"},{6379,"redis"},{8080,"http-alt"},
    {8443,"https-alt"},{9100,"printer"},
};

const uint16_t COMMON_PORTS[] = {
    21,22,23,25,53,80,110,135,139,143,161,389,443,445,
    587,993,995,1433,3306,3389,5432,5900,6379,8080,8443,9100,
};
const size_t COMMON_PORTS_N = sizeof(COMMON_PORTS) / sizeof(COMMON_PORTS[0]);

const char* portName(uint16_t port)
{
    for (auto& e : PORT_TABLE) if (e.port == port) return e.name;
    return "";
}

bool resolve(const String& host, IPAddress& out)
{
    // Already an IP literal?
    if (out.fromString(host)) return true;
    return WiFi.hostByName(host.c_str(), out) == 1;
}

void portScan(Worker& w, IPAddress host, const uint16_t* ports, size_t n, uint32_t timeoutMs)
{
    w.emit("Scanning " + host.toString() + " (" + String((unsigned)n) + " ports)", theme::ACCENT);
    int open = 0;
    for (size_t i = 0; i < n && !w.stopRequested(); ++i) {
        uint16_t p = ports[i];
        WiFiClient c;
        bool isOpen = c.connect(host, p, (int32_t)timeoutMs);
        c.stop();
        if (isOpen) {
            ++open;
            const char* nm = portName(p);
            String line = String((int)p);
            if (nm[0]) line += String(" (") + nm + ")";
            line += "  OPEN";
            w.emit(line, theme::OK);
        }
    }
    if (w.stopRequested()) w.emit("-- stopped --", theme::WARN);
    else                   w.emit("Done. " + String(open) + " open.", theme::ACCENT);
}

// ---- ICMP ping via esp_ping ------------------------------------------------
struct PingCtx {
    Worker* w;
    SemaphoreHandle_t done;
};

static void onSuccess(esp_ping_handle_t h, void* args)
{
    PingCtx* c = static_cast<PingCtx*>(args);
    uint32_t seq = 0, ttl = 0, elapsed = 0;
    esp_ping_get_profile(h, ESP_PING_PROF_SEQNO,   &seq,     sizeof(seq));
    esp_ping_get_profile(h, ESP_PING_PROF_TTL,     &ttl,     sizeof(ttl));
    esp_ping_get_profile(h, ESP_PING_PROF_TIMEGAP, &elapsed, sizeof(elapsed));
    c->w->emit("reply seq=" + String(seq) + " ttl=" + String(ttl) +
               " " + String(elapsed) + "ms", theme::OK);
}

static void onTimeout(esp_ping_handle_t h, void* args)
{
    PingCtx* c = static_cast<PingCtx*>(args);
    uint32_t seq = 0;
    esp_ping_get_profile(h, ESP_PING_PROF_SEQNO, &seq, sizeof(seq));
    c->w->emit("seq=" + String(seq) + " timeout", theme::WARN);
}

static void onEnd(esp_ping_handle_t h, void* args)
{
    PingCtx* c = static_cast<PingCtx*>(args);
    uint32_t tx = 0, rx = 0, total = 0;
    esp_ping_get_profile(h, ESP_PING_PROF_REQUEST,  &tx,    sizeof(tx));
    esp_ping_get_profile(h, ESP_PING_PROF_REPLY,    &rx,    sizeof(rx));
    esp_ping_get_profile(h, ESP_PING_PROF_DURATION, &total, sizeof(total));
    uint32_t loss = tx ? (100 * (tx - rx) / tx) : 100;
    c->w->emit(String(tx) + " sent " + String(rx) + " recv " +
               String(loss) + "% loss", theme::ACCENT);
    xSemaphoreGive(c->done);
}

void pingHost(Worker& w, IPAddress host, int count)
{
    w.emit("PING " + host.toString(), theme::ACCENT);

    esp_ping_config_t cfg = ESP_PING_DEFAULT_CONFIG();
    ip_addr_t target;
    memset(&target, 0, sizeof(target));
    target.type = IPADDR_TYPE_V4;
    target.u_addr.ip4.addr = (uint32_t)host;   // matches lwip network byte order
    cfg.target_addr = target;
    cfg.count       = count;
    cfg.interval_ms = 500;
    cfg.timeout_ms  = 1000;

    PingCtx ctx{&w, xSemaphoreCreateBinary()};
    if (!ctx.done) { w.emit("ping: no mem", theme::ERR); return; }

    esp_ping_callbacks_t cbs = {};
    cbs.cb_args          = &ctx;
    cbs.on_ping_success  = onSuccess;
    cbs.on_ping_timeout  = onTimeout;
    cbs.on_ping_end      = onEnd;

    esp_ping_handle_t h = nullptr;
    if (esp_ping_new_session(&cfg, &cbs, &h) != ESP_OK || !h) {
        w.emit("ping: session failed", theme::ERR);
        vSemaphoreDelete(ctx.done);
        return;
    }
    esp_ping_start(h);

    // Wait for completion, but wake periodically so a stop request is honoured.
    while (xSemaphoreTake(ctx.done, pdMS_TO_TICKS(200)) == pdFALSE) {
        if (w.stopRequested()) { esp_ping_stop(h); }
    }
    esp_ping_delete_session(h);
    vSemaphoreDelete(ctx.done);
}

} // namespace nettools
