#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct cpuusage {
    char name[20];
    // Absolute values since last reboot.
    unsigned long long idletime;
    unsigned long long workingtime;
};

struct cpustat {
    char name[20];
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

struct cpuusage cpuusage_from_cpustat(struct cpustat s) {
    struct cpuusage r;
    strncpy(r.name, s.name, sizeof(r.name));
    r.name[sizeof(r.name) - 1] = '\0';
    r.idletime = s.idle + s.iowait;
    r.workingtime = s.user + s.nice + s.system + s.irq + s.softirq;
    return r;
}

void cpuusage_show_diff(struct cpuusage now, struct cpuusage prev) {
    // the number of ticks that passed by since the last measurement
    const unsigned long long workingtime = now.workingtime - prev.workingtime;
    const unsigned long long alltime = workingtime + (now.idletime - prev.idletime);
    // they are divided by themselves - so the unit does not matter.
    printf("Usage: %.0Lf%%\n", (long double)workingtime / alltime * 100.0L);
}

int main() {
    struct cpuusage prev = {0};
    //
    const int stat = open("/proc/self/stat", O_RDONLY);
    assert(stat != -1);
    fcntl(stat, F_SETFL, O_NONBLOCK);
    while (1) {
        // let's read everything in one call so it's nicely synced.
        int r = lseek(stat, SEEK_SET, 0);
        assert(r != -1);
        char buffer[10001];
        const ssize_t readed = read(stat, buffer, sizeof(buffer) - 1);
        assert(readed != -1);
        buffer[readed] = '\0';
        // Read the values from the readed buffer/
        FILE *f = fmemopen(buffer, readed, "r");
        // Uch, so much borign typing.
        struct cpustat c = {0};
        while (fscanf(f, "%19s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", c.name, &c.user, &c.nice,
                  &c.system, &c.idle, &c.iowait, &c.irq, &c.softirq, &c.steal, &c.guest,
                  &c.guest_nice) == 11) {
            // Just an example for first cpu core.
            if (strcmp(c.name, "cpu0") == 0) {
                struct cpuusage now = cpuusage_from_cpustat(c);
                cpuusage_show_diff(now, prev);
                prev = now;
                break;
            }
        }
        fclose(f);
        //
        sleep(1);
    }
}
