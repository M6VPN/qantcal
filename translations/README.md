# Translations

qantcal uses Qt Linguist translation files.

To add or update a translation:

1. Add or edit `qantcal_<locale>.ts` in this directory.
2. Add the file to `QANTCAL_TRANSLATION_SOURCES` in the root `CMakeLists.txt`.
3. Build normally. CMake runs `lrelease` and installs the generated `.qm` file.

Translation files are source files. Commit the `.ts` file, not generated `.qm` files.

The application searches these paths at startup:

- `QANTCAL_TRANSLATION_DIR`, if set
- `translations/` next to the executable
- `../translations/` from the executable directory
- `../share/qantcal/translations/` from the executable directory
- `translations/` in the current working directory
