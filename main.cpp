#include "mbed.h"
#include "TCPSocket.h"

#define STR(x) STR2(x)
#define STR2(x) #x


// Socket demo
void http_demo(NetworkInterface *net) {
    TCPSocket socket;

    // Show the network address
    const char *ip = net->get_ip_address();
    printf("IP address is: %s\r\n", ip ? ip : "No IP");

    // Open a socket on the network interface, and create a TCP connection to mbed.org
    socket.open(net);
    socket.connect("developer.mbed.org", 80);

    // Send a simple http request
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: developer.mbed.org\r\n\r\n";
    int scount = socket.send(sbuffer, sizeof sbuffer);
    printf("sent %d [%.*s]\r\n", scount, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);

    // Recieve a simple http response and print out the response line
    char rbuffer[64];
    int rcount = socket.recv(rbuffer, sizeof rbuffer);
    printf("recv %d [%.*s]\r\n", rcount, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);

    // Close the socket to return its memory and bring down the network interface
    socket.close();
}


// Example with the ESP8266 interface
#if defined(MBED_DEMO_WIFI)
#include "ESP8266Interface.h"

void connect_cb(nsapi_error_t res, void *data)
{
    Semaphore *s = (Semaphore*)data;
    printf("GOT CB: connect! error: %d\r\n", res);
    s->release();
}

void scan_cb(wifi_ap_t *ap, void *data)
{
    Semaphore *s = (Semaphore*)data;
    printf("GOT CB: AP \r\n");
    if (ap) {
    printf("Found: %s, sec: %hhu, ch %hhu, rssi %hd, %X:%X:%X:%X:%X:%X\r\n", ap->ssid, ap->security,
               ap->channel, ap->rssi, ap->bssid[0], ap->bssid[1],ap->bssid[2],ap->bssid[3],ap->bssid[4],ap->bssid[5]);
    } else {
        s->release();
    }
}

ESP8266Interface wifi(D1, D0);

int main() {
    // Brings up the esp8266
    printf("ESP8266 socket example\r\n");
    Semaphore wait_connected;

#if 0
    wifi.connect(STR(MBED_DEMO_WIFI_SSID), STR(MBED_DEMO_WIFI_PASS));

    int aps = wifi.scan((wifi_ap_t *)NULL, 0);
    printf("SCAN: %d\r\n", aps);
    wifi_ap_t *res = new wifi_ap_t[aps];
    aps = wifi.scan(res, aps);
    for (int i = 0; i < aps; i++) {
        printf("Found: %s, sec: %hhu, ch %hhu, rssi %hd, %X:%X:%X:%X:%X:%X\r\n", res[i].ssid, res[i].security,
               res[i].channel, res[i].rssi, res[i].bssid[0], res[i].bssid[1],res[i].bssid[2],res[i].bssid[3],res[i].bssid[4],res[i].bssid[5]);
    }
    free(res);
#else
    wifi.connect_async(STR(MBED_DEMO_WIFI_SSID), STR(MBED_DEMO_WIFI_PASS), NSAPI_SECURITY_NONE, connect_cb, &wait_connected);
    wait_connected.wait();
    wifi.scan_async(scan_cb, &wait_connected);
    wait_connected.wait();
#endif
    printf("RSSI: %d\r\n", wifi.get_rssi());

    // Invoke the demo
    http_demo(&wifi);

    // Brings down the esp8266
    wifi.disconnect();

    printf("Done\r\n");
}

// Example using the builtin ethernet interface
#else
#include "EthernetInterface.h"

EthernetInterface eth;

int main() {
    // Brings up the ethernet interface
    printf("Ethernet socket example\r\n");
    eth.connect();

    // Invoke the demo
    http_demo(&eth);

    // Brings down the ethernet interface
    eth.disconnect();

    printf("Done\r\n");
}
#endif
