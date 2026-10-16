# zmk-behavior-auto-repeat

Repeats keys and mouse buttons at fixed interval while you keep typing.
Hold `&auto_repeat` and press keys to start or stop their repeat, or press keys on layers
listed in `layers`.
Speed comes from two keymap commands or encoder.

## Install

Add it to your `config/west.yml`:
```yaml
  remotes:
    - name: damex
      url-base: https://github.com/damex
  projects:
    - name: zmk-behavior-auto-repeat
      remote: damex
      revision: v0.1.0
```
Then `west update`.
Local checkout: `-DZMK_EXTRA_MODULES=<path>/zmk-behavior-auto-repeat`.

## Configure

Include predefined behaviors after `behaviors.dtsi` in your keymap file:
```dts
#include <behaviors.dtsi>
#include <behaviors/auto_repeat.dtsi>
```
Bind `&auto_repeat` on any key:
```dts
&auto_repeat
```
- Hold `&auto_repeat` and press key: that key starts repeating.
- Hold it and press repeating key: that key stops.
- Release `&auto_repeat`: keys type normally while picked keys keep repeating.
- Tap `&auto_repeat` alone: every key stops.
- Modifiers held at pick time repeat with key, so holding `&auto_repeat` and Shift while
  pressing A repeats Shift+A.
  `&mkp` buttons repeat without modifiers.
- Repeat keeps binding from pick time, whatever layer is active later.

Keys bound to `&kp`, modifiers aside, and buttons bound to `&mkp` get picked.
Other bindings work as usual.
`&mkp` needs pointing in your `.conf`:
```conf
CONFIG_ZMK_POINTING=y
```

To pick from layers instead, list them in `layers`.
While any listed layer is active, every key press starts or stops repeat, as with
`&auto_repeat` held.
Numpad on layer 1, turned on and off with `&tog 1`:
```dts
&auto_repeat {
    layers = <1>;
};
```
```dts
numpad_layer {
    bindings = <
        &kp KP_N7 &kp KP_N8 &kp KP_N9  &kp KP_MINUS
        &kp KP_N4 &kp KP_N5 &kp KP_N6  &kp KP_PLUS
        &kp KP_N1 &kp KP_N2 &kp KP_N3  &kp KP_ENTER
        &tog 1    &kp KP_N0 &kp KP_DOT &kp KP_SLASH
    >;
};
```

Change speed of every repeat from two keys:
```dts
&auto_repeat_speed AUTO_REPEAT_FASTER
&auto_repeat_speed AUTO_REPEAT_SLOWER
```
Or from encoder, clockwise faster and counter-clockwise slower:
```dts
sensor-bindings = <&auto_repeat_encoder>;
```

Tune `&auto_repeat` in your keymap file:
```dts
&auto_repeat {
    interval-ms = <30>;
};
```

| property | default | description |
|---|---|---|
| `interval-ms` | 50 | press to press time at boot |
| `tap-ms` | 10 | hold time per tap |
| `interval-step-ms` | 10 | interval change per faster or slower command |
| `interval-min-ms` | 20 | shortest interval, above `tap-ms` |
| `interval-max-ms` | 500 | longest interval |
| `max-active` | 4 | keys repeating at once |
| `layers` | none | layers on which every key press starts or stops repeat |

| command | effect |
|---|---|
| `AUTO_REPEAT_FASTER` | interval minus one step, down to `interval-min-ms` |
| `AUTO_REPEAT_SLOWER` | interval plus one step, up to `interval-max-ms` |

One interval drives every repeating key.
Speed change moves each key's next tap.

## Limitations

- Picking press also types key, as first tap.
  Picking repeating key again types it once more as it stops.
  ZMK runs this module's key listener after keymap.
- With `max-active` keys repeating, another pick types normally and logs warning.
- Interval resets to `interval-ms` on reboot.
- Modifiers held by hand apply to timed taps, so Ctrl held while W repeats sends Ctrl+W.
- Holding repeating key by hand overlaps its taps.
  Timed release ends hand hold early.
  Hand release ends timed tap early.
- Central or non-split only.
  In split, ZMK runs keymap on central, so keys on peripheral half still get picked.
  `CONFIG_ZMK_BEHAVIOR_AUTO_REPEAT` gates build, off on split peripherals.
- Taps are no activity for ZMK, which counts key presses and sensors only.
  With `CONFIG_ZMK_SLEEP`, device sleeps `CONFIG_ZMK_IDLE_SLEEP_TIMEOUT` after last key press
  even while keys repeat.

## License

This module is MIT.

Dependencies (each keeps its own license):

| Dependency | License |
|---|---|
| ZMK | MIT |
| Zephyr | Apache-2.0 |
