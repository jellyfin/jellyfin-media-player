SPMediaKeyTap
=============

`SPMediaKeyTap` abstracts a `CGEventHook` and other nastiness in order to give you a relatively simple API to receive media key events (prev/next/playpause, on F7 to F9 on modern MacBook Pros) exclusively, without them reaching other applications like iTunes. `SPMediaKeyTap` is clever enough to resign its exclusive lock on media keys by looking for which application was active most recently: if that application is in `SPMediaKeyTap`'s whitelist, it will resign the keys. This is similar to the behavior of Apple's applications collaborating on media key handling exclusivity, but unfortunately, Apple is not exposing any APIs allowing third-parties to join in on this collaboration.

For now, the whitelist is just a hardcoded array in `+[SPMediaKeyTap defaultMediaKeyUserBundleIdentifiers]`. If your app starts using `SPMediaKeyTap`, please [mail me](mailto:nevyn@spotify.com) your bundle ID, and I'll include it in the canonical repository. This is a bad solution; a better solution would be to use distributed notifications to collaborate in creating this whitelist at runtime. Hopefully someone'll have the time and energy to write this soon. 

In `Example/SPMediaKeyTapExampleAppDelegate.m` is an example of both how you use `SPMediaKeyTap`, and how you handle the semi-private `NSEvent` subtypes involved in media keys, including on how to fall back to non-event tap handling of these events.

`SPMediaKeyTap` and other `CGEventHook`s on the event type `NSSystemDefined` is known to interfere with each other and applications doing weird stuff with mouse input, because mouse clicks are also part of the `NSSystemDefined` category. The single issue we have had reported here at Spotify is Adobe Fireworks, in which item selection stops working with `SPMediaKeyTap` is active.

`SPMediaKeyTap` requires 10.5 to work, and disables itself on 10.4.
