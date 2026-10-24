# 3D Maze (2016)

A 3D maze game in C++ with OpenGL and GLUT, made by a team of three as a university course project in December 2016.

Shared as a record of a student team project. Parts come from the SuperSDG skeleton (c) Ravent; no licence is granted.

## What the code does

`src/ET_Maze.cpp` is the version the team submitted.

- A console title screen in box-drawing characters waits for a key, then the game opens a full-screen GLUT window.
- Every run generates a new random 15 x 15 maze. Walls are added at random only while every open cell stays reachable, which is checked with a flood fill.
- Walls and floor are textured boxes and quads under one light. The player, two items, an exit and 50 spinning batteries are placed on random open cells; the exit is put near the far end of a breadth-first fill so it is a long way from its starting point.
- The player is a 3D model loaded from OBJ files by a small OBJ loader. While the player moves, the code loads the next frame of a sequence of OBJ files, which makes the model walk.
- The game starts in a first-person view. Picking up the first item switches to an overhead view and unlocks it on key `2`; the second item switches to a view of the whole maze and unlocks it on key `3`. Key `1` returns to first person, and key `0` shows the whole maze at any time (it looks like a testing shortcut).
- Arrow keys move. In first person, Left and Right turn and Up moves forward; in the other views the arrows move in four directions.
- Each battery collected adds 100 points. Reaching the exit closes the window and prints an "END" banner with the elapsed seconds and the score. `Q` or `Esc` quits.

`prototype/3D_MazeGame.cpp` is an earlier version of the same program: a 40 x 40 maze, an animated OBJ model as the player, a red teapot as the exit, `A` and `S` to rotate the camera, `Z` and `X` to zoom and `R` to reset. Comparing the two files shows what was added for the final game (items, batteries, view modes, score, timer and the console screens).

## Facts

| | |
|---|---|
| Year | 2016 |
| Team | 3 students |
| Dawn's part | Game programming, project management and the presentation. The other two members made the player model and its animation, and the game objects. |
| Language | C++ (Windows) |
| Original toolchain | Visual Studio 2015 (platform toolset `v140`; the solution was first created in Visual Studio 2013), GLUT for Win32, GLAux |
| Libraries linked | `glut32.lib`, `glaux.lib`, `legacy_stdio_definitions.lib` (via `#pragma comment`) |

## Third-party code in these files

- **SuperSDG skeleton.** Both `.cpp` files start with the header "SuperSDG v2.2 release / Copyright (c) 2002-2008 Ravent / All rights reserved". The header is kept exactly as it was. The program was built on that skeleton, and no licence for it is known, so no licence is granted for these files.
- **OBJ loader.** `loadObj()` and its comments appear to follow a widely shared OBJ loader tutorial. Its origin has not been confirmed.
- **GLUT and GLAux.** Headers and libraries from those projects were used but are not included here.

## Building it today

This has not been tried. The code is Windows-only as written (`conio.h`, `_getch`, `sscanf_s`, wide-string `auxDIBImageLoad`).

1. Use Visual Studio 2022 with the C++ desktop workload, and replace GLUT for Win32 with freeglut (for example through vcpkg).
2. GLAux is obsolete and no longer shipped. Replace `auxDIBImageLoad` with another image loader (for example a small BMP loader or stb_image) that fills the same RGB buffer for `gluBuild2DMipmaps`.
3. The source is UTF-8. Compile with `/utf-8` so the Korean strings and box-drawing characters stay correct.
4. Supply the files the program opens from its working directory:
   - textures `wall.bmp`, `floor.bmp`, `item.bmp`, `batterys.bmp`, `wayout.bmp`
   - player model frames `main1.obj` to `main70.obj`, plus a header `playerObj.h` that lists them: `char *playerObj[70] = { "main1.obj", ..., "main70.obj" };`
   - for the prototype, `1.bmp`, `2.bmp`, the frames named in `zombieName.h`, and that header

## What was removed and why

| Removed | Why |
|---|---|
| Player and zombie OBJ/MTL models, `playerObj.h`, `zombieName.h` | The models are a teammate's work or of unknown origin; the two headers only list the model file names |
| Texture bitmaps (`wall`, `floor`, `item`, `batterys`, `wayout`, `1`, `2`) | Stock photos and clip art of unknown origin, not clearly the team's own |
| `GLUT.H`, `GLUT32.DLL`, `GLUT32.LIB`, `GLUT.DEF`, `GLAux.h`, `Glaux.lib` | Third-party headers and binaries |
| `ET_Maze.exe` | Build output |
| Presentation, report and role documents (PDF, PPTX, ZIP) | Office and PDF files contain personal details |
| `.sln`, `.vcxproj`, `.VC.db`, `.sdf`, `.opensdf`, `.suo` | Project and IDE cache files |
| `SourceCode.cpp` | A reformatted duplicate of `prototype/3D_MazeGame.cpp` |

Changes to the code: `src/ET_Maze.cpp` was converted from CP949 (EUC-KR) to UTF-8, and the two teammates' names in the title screen credits were replaced with `(팀원)` ("team member"). Nothing else was changed. `prototype/3D_MazeGame.cpp` is unchanged.

## Status

Archived student team project from 2016. Not maintained. It does not build as published: the models, textures and libraries are missing.
