#include "abscom/abs.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int login_calls = 0;
static var last_login = NULL;

static var on_user_login(var user) {
    (void)user;
    login_calls++;
    last_login = user;
    return None;
}

static int audit_calls = 0;

static var on_audit(var data) {
    (void)data;
    audit_calls++;
    return None;
}

int main(void) {
    abs_init();

    var bus = EventBus();
    CHECK(bus != NULL);
    CHECK(bus->type == ABS_DICT);

    /* Emitting an event with no handlers is a no-op. */
    emit(bus, "login", v("nobody"));
    CHECK(login_calls == 0);

    /* Registered handlers run with the emitted payload. */
    on(bus, "login", on_user_login);
    on(bus, "login", on_user_login);
    emit(bus, "login", v("Alice"));
    CHECK(login_calls == 2);
    CHECK(last_login != NULL);
    CHECK(last_login->type == ABS_STR);
    CHECK(strcmp(last_login->val.s, "Alice") == 0);

    /* Different events don't leak into each other. */
    on(bus, "audit", on_audit);
    emit(bus, "audit", v(1));
    CHECK(audit_calls == 1);
    CHECK(login_calls == 2); /* unchanged */

    abs_cleanup();
    return 0;
}
