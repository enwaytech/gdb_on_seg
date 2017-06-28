# gdb_on_seg
Preload code for segfault handling which launches GDB automagically

Originally taken from https://stackoverflow.com/a/25499817 (https://stackoverflow.com/a/22509089)


# Usage

- Compile by running ``./compile``
- Result ``.so`` file is in ``lib`` folder
- Add LD_PRELOAD and run executable: ``LD_PRELOAD=<path to build folder>/gdb_on_seg.so executable``
- Or ``export LD_PRELOAD=<path to build folder>/gdb_on_seg.so`` and then run the executable normally
