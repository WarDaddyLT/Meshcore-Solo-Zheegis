#pragma once

#include "Mesh.h"
#include <stddef.h>
#include <stdio.h>


class LocationProvider {
protected:
    bool _time_sync_needed = true;

public:
    virtual void syncTime() { _time_sync_needed = true; }
    virtual bool waitingTimeSync() { return _time_sync_needed; }
    virtual long getLatitude() = 0;
    virtual long getLongitude() = 0;
    virtual long getAltitude() = 0;
    virtual long satellitesCount() = 0;
    // Horizontal Dilution of Precision, in tenths (11 == HDOP 1.1) -- lower is
    // better, and a much more direct read on fix quality than satellite count
    // alone (few satellites in good geometry can beat many in poor geometry).
    // -1 means this provider doesn't expose it; callers fall back to
    // satellitesCount() in that case.
    virtual long getHDOP() { return -1; }
    virtual bool isValid() = 0;
    virtual uint32_t getFixGeneration() const { return 0; }
    virtual bool hasFreshFix(uint32_t max_age_ms) { return false; }
    virtual long getTimestamp() = 0;
    // Default no-op body (every existing subclass overrides this anyway --
    // MicroNMEALocationProvider.h forwards to the NMEA lib,
    // EnvironmentSensorManager.cpp's GPS classes no-op it explicitly) --
    // previously declared with NO definition anywhere in the codebase
    // (confirmed by repo-wide grep), which is latently a hard link error
    // ("undefined symbol: typeinfo for LocationProvider") for ANY subclass
    // that doesn't override it, since base-subobject construction of any
    // LocationProvider-derived object needs this class's own vtable/RTTI to
    // exist as a real linkable symbol. Surfaced by variants/sim's
    // SimLocationProvider (Phase 3 of the sim plan) being the first
    // consumer to construct one outside of code that always overrides it.
    virtual void sendSentence(const char * sentence) { }
    virtual void drain() {}
    virtual void reset() = 0;
    virtual void begin() = 0;
    virtual void stop() = 0;
    virtual void loop() = 0;
    virtual bool isEnabled() = 0;
    // Format compact diagnostics in the caller-provided buffer. Providers that
    // have more information (for example UART counters) can override this.
    virtual void formatDiagnostics(char* out, size_t out_size) {
        if (out_size == 0) return;
        snprintf(out, out_size, "en:%u sat:%ld fix:%u",
            isEnabled() ? 1U : 0U,
            satellitesCount(),
            isValid() ? 1U : 0U);
    }
};
