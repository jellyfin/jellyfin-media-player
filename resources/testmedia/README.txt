Produced with:

    x264 input.png --profile=high --tune=stillimage --level=5.1 -o out.h264

where input.png was a black image with the appropriate resolution.

x264 was patched to remove the SEI message (because it doubled the size).
