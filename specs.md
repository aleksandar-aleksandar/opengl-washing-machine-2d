Evo prevoda kompletne specifikacije na engleski, uz očuvanu originalnu strukturu, numeraciju i Markdown formatiranje. Terminologiju sam prilagodio OpenGL/C++ kontekstu.

Markdown document

# 2D Project Specifications 2026/2027

## General Information (Also Applies to Independent Topics!)

The defense of the 2D project will take place during lab sessions in the week of **November 16–20**. Each student must attend the defense at the designated time corresponding to their assigned group, unless they have informed the teaching assistant during lab sessions that they have swapped places with another student.

The project must be implemented in **C++**, using the OpenGL library.

**The OpenGL version must be at least 3.0, and the programmable pipeline must be used! Projects implemented using the fixed-function pipeline will not be graded!**

The project must include the following techniques covered during lab sessions: shader programming, uniforms, transparent colors and/or textures, mouse and/or keyboard input, and a cursor with an attached image (for independent topics, the cursor must resemble something related to the topic, e.g. an ice cube for a refrigerator).

Any text or numbers displayed on the screen can be implemented using textures or the FreeType library.

Whenever constant-speed movement is required, implementing accelerated movement instead is also acceptable if the student prefers it (the reverse does not apply!).

**The project must include a frame limiter set to 75 FPS, run in full-screen mode, and allow the user to exit at any time by pressing the Escape key.** A semi-transparent texture containing the student's first name, last name, and student ID number must be displayed in any corner of the screen.

Students who have proposed independent topics must contact the teaching assistant by email (if they have not already done so) at [vasilijem@uns.ac.rs](mailto:vasilijem@uns.ac.rs) to get their topics approved.

The [spreadsheet available at this link](https://docs.google.com/spreadsheets/d/1bkQsxFHKnOTYth4ZEpYlxrmebQHCfMnx5_37-du3srw/edit?usp=sharing) lists the students who have independent topics, along with their respective topic titles.

The 2D project must be completed by the scheduled defense date. A project is considered passed if **more than 50% of the available points** are earned.

The following section describes 10 predefined topics. Each student is assigned the topic corresponding to the last digit of their student ID number, excluding the enrollment year (e.g. a student with ID SV 15/2021 would be assigned topic number 5).

**Students defending their projects on university computers must submit their work in `.zip` format or provide a GitHub link to [vasilijem@uns.ac.rs](mailto:vasilijem@uns.ac.rs) at least 30 minutes before their scheduled defense!**

**Students who have not been assigned to a group must indicate which defense session they can attend by emailing [vasilijem@uns.ac.rs](mailto:vasilijem@uns.ac.rs).**

---

# 0. Washing Machine

Create a 2D graphics application that simulates the operation of a washing machine.

When the program starts, a washing machine with its door open and a circular compartment for loading clothes should be displayed on the screen. On the left side of the screen, there should be at least 3 different white clothing items and exactly one red clothing item.

By dragging any clothing item to the washing machine using the left mouse button, the user should be able to load it into the machine. Once loaded, the item is positioned inside the circular compartment and can no longer be dragged.

At any time, if there is at least one clothing item inside the machine, the user can press **ENTER** to start the washing cycle. When this happens, all clothing items that have not been loaded into the machine disappear from the screen.

When the washing cycle starts, the door closes and moves in front of the compartment containing the clothes. The door must have a semi-transparent dark gray layer through which the clothes remain visible.

Once the door has closed, the compartment containing the clothes begins rotating clockwise with increasing speed **(only the clothes inside it should move)** until it reaches a certain speed. It then continues rotating at that speed for **20 seconds**.

Each clothing item moves along with the rotating compartment as follows: it follows a circular path until it reaches the topmost point of the path. At that point, it begins falling vertically downward with increasing speed until it hits the bottom edge of the compartment. Once it hits the bottom edge, the movement cycle starts again, repeating until the washing machine stops rotating.

If the red clothing item is being washed, all white clothing items inside the machine gradually turn pink during the washing cycle (if any are present).

When the washing cycle is complete, the washing machine door opens, revealing all the clothing items. The only action available to the user at this point is to exit the program.

The cursor must have the appearance of a **fabric softener bottle**.