#include <spectrogram/platform.hpp>
#import <Cocoa/Cocoa.h>

static id g_observer = nil;

void maximizeMacWindow(sf::WindowHandle handle) {
    NSWindow* window = (__bridge NSWindow*) handle;
    if (![window isZoomed]) [window zoom:nil];
}

void install_live_resize_hook(sf::WindowHandle handle, std::function<void()> on_resize) {
    NSWindow* window = (__bridge NSWindow*) handle;
    auto cb = sd::make_shared<std::function<void()>>(std::move(on_resize));

    g_observer = [[NSNotificationCenter defaultCenter]
        addOberverForName:NSWindowDidResizeNotification
                   object:window
                    queue:nil
               usingBlock:^(NSNotification*) { (*cb)(); }];
}

void remove_live_resize_hook(sf::WindowHandle) {
    if (g_observer) {
        [[NSNotificationCenter defaultCenter] removeObserver:g_observer];
        g_observer = nil;
    }
}
