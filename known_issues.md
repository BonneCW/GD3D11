# Known Issues

## General

- After resizing the application with the F11 menu
the game does not properly respond to mouse input,
the Inventory is out of screen, ...
    - Workaround: Apply(\*) the resolution change and restart the game.

## G1

- Incompatible with `Systempack.ini` Setting `Animated_Inventory`
    - Workaround: Set `Animated_Inventory=0` (disable it)
- Items used by NPCs in animations (Smith, cook, mining) duplicate infititely

## G2

- Shadows on Trees and other things flicker
- Geometry drawn with `zCRndD3D::Drawline` / `zCRndD3D::DrawlineZ` flickers aswell
