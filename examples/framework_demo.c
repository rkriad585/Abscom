#include "abscom/abs.h"

#include <stdio.h>
#include <string.h>

/* --- 1. Web handlers --- */
static var api_home(var req) {
    (void)req;
    return v("<h1>Abscom Framework</h1><p>Hello from the C web server!</p>");
}

static var api_about(var req) {
    (void)req;
    return v("{\"framework\": \"abscom\", \"version\": \"0.1.0\"}");
}

/* --- 2. Event handlers --- */
static int login_count = 0;

static var on_user_login(var user) {
    login_count++;
    print(v("  user logged in:"), user);
    return None;
}

/* --- 3. Memoized computation --- */
static int fib_calls = 0;
static var current_fib = NULL; /* set in main() to memoize(fib_n) */

static var fib_n(var n) {
    fib_calls++;
    if (n->val.i < 2) return v(n->val.i);
    var a = call_func(current_fib, v(n->val.i - 1));
    var b = call_func(current_fib, v(n->val.i - 2));
    return v(a->val.i + b->val.i);
}

/* --- 4. Decorator example --- */
static var shout(var s) {
    (void)s;
    return v("SHOUT");
}

int main(int argc, char **argv) {
    abs_init();

    /* --- 1. Web server (routes work without a socket too) --- */
    print(v("--- 1. Web server ---"));
    var app = Server(0);
    route(app, "/", api_home);
    route(app, "/about", api_about);

    var body = server_handle(app, "GET / HTTP/1.1");
    if (body) {
        print(v("GET / ->"), body);
    } else {
        print(v("GET / -> 404"));
    }
    body = server_handle(app, "GET /about HTTP/1.1");
    if (body) {
        print(v("GET /about ->"), body);
    } else {
        print(v("GET /about -> 404"));
    }
    if (!server_handle(app, "GET /nope HTTP/1.1")) {
        print(v("GET /nope -> 404"));
    }
    print(v("Server type:"), type(app));

    /* Start the blocking HTTP server only when asked to. */
    if (argc > 1 && strcmp(argv[1], "--server") == 0) {
        server_run(app);
        abs_cleanup();
        return 0;
    }

    /* --- 2. Event bus --- */
    print(v("--- 2. Event bus ---"));
    var bus = EventBus();
    on(bus, "login", on_user_login);
    on(bus, "login", on_user_login);
    emit(bus, "login", v("Alice"));
    print(v("  handlers run:"), v(login_count));

    /* --- 3. Memoized fibonacci --- */
    print(v("--- 3. Memoized fib ---"));
    current_fib = memoize(fib_n);
    var fib10 = call_func(current_fib, v(10));
    print(v("  fib(10) ="), fib10);
    print(v("  body calls made:"), v(fib_calls));
    var fib10_again = call_func(current_fib, v(10));
    print(v("  fib(10) again ="), fib10_again);
    print(v("  body calls after cache hit:"), v(fib_calls));

    /* --- 4. Decorators --- */
    print(v("--- 4. Decorator ---"));
    var plain = make_func(on_user_login);
    var loud = decorate(plain, shout);
    print(v("  decorated call ->"), call_func(loud, None));

    /* --- 5. Introspection --- */
    print(v("--- 5. Introspection ---"));
    var d = Dict();
    dset(d, "name", v("abscom"));
    dset(d, "level", v(9));
    print(v("  dir(dict):"), dir(d));
    print(v("  id(d):"), id(d));
    print(v("  repr(d):"), repr(d));

    /* --- 6. itertools --- */
    print(v("--- 6. itertools ---"));
    var nums = List();
    append(nums, v(1));
    append(nums, v(2));
    var more = List();
    append(more, v(3));
    append(more, v(4));
    var chained = chain(nums, more);
    print(v("  chain:"), iter_next(chained), iter_next(chained),
          iter_next(chained), iter_next(chained));
    var cycled = cycle(nums);
    print(v("  cycle:"), iter_next(cycled), iter_next(cycled),
          iter_next(cycled), iter_next(cycled));

    abs_cleanup();
    return 0;
}
