# Steno Atlas

![Atlas Screenshot](https://github.com/user-attachments/assets/a7768d38-7dcc-43cf-98f7-e421ecc4d630)

The steno Atlas is a 2-D visualization tool for stenographic dictionary files. The prototype is live at [zstolfi.github.io/steno/atlas](https://zstolfi.github.io/steno/atlas/).

## Getting Started

To start, drag and drop a dictionary file onto the site. If you have an unsupported dictionary file, try exporting it as a TXT.
Alternatively, you can browse Plover's default dictionary, `main.json`.

TXT and JSON files are currently supported, with limited RTF support. If an RTF upload doesn't work, try exporting it as a TXT file.

## Reading the Atlas

The Atlas is a map of single-stroke dictionary entries, where black regions are unused space. This makes it a great way to scout out locations for new left-hand phrase starters, or right-hand phrase enders.

By default, entries are displayed clustered by starting keys. This means words which start with the same letter or phrase appear close-by.
In this view entries are colored by starting letter.

Furthermore, every possible starting prefix of a stroke is mapped to a unique rectangle. So in Magnum Steno, every entry that begins with `SKP, "and"` appears as a big red rectangle which takes up 1/16th of the Atlas.

## How it Works

The Atlas uses a [Hilbert curve](https://en.wikipedia.org/wiki/Hilbert_curve) to map every possible combination of 22 steno keys onto a unique position on a 2048 × 2048 image. The idea is to convert a steno stroke into a binary number, and find where that number lives on the 11th iteration Hilbert curve.

For example, take the stroke `SPROUTS`:
```c++
STKPWHRAOEU*FRPBLGTSDZ // accepted keys (no number bar just yet)
S  P  R O U       TS   // stroke subset
1001001010100000001100 // converted number
```
This is then fed as input into the `hilbert(uint)` function, which returns {1251, 1761}.

Doing this for every entry in a dictionary generates our Atlas image.

<details>
<summary>[Aside]</summary>
Note that the asterisk is grouped with the right-side consonants, as opposed to being sandwiched between the `O` and `E` keys like it is with regular keyboards. This is because the key is pressed with the right hand, so it allows right hand phrase enders to be displayed as contiguous regions.
</details>

## Alternate View

Currently only two views (perspectives) of the data are available. Both of which are hilbert curves. The default view clusters by prefix ("met, meant, mend, mends"), the second view clusters by suffix ("met, set, fret, sweat").

The second view is achieved by reversing the bit ordering before sending input into the `hilbert()` function.
