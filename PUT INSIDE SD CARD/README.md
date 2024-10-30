### Instructions for Setting Up Your SD Card

1. **Copy Essential Files**  
   Begin by copying the contents from the **"/SD CARD MUST HAVE!"** folder to your SD card.

2. **Adding Programs**  
   You can add your programs to the **"Programs"** folder. To do this, create a new subfolder within the **"Programs"** folder.

3. **Firmware and HTML Files**  
   Place your **firmware.bin** file in the new subfolder. If you would like to include a preview or explanation for your program, create an **index.html** file and add it to the same folder.

4. **Using LoaderLib**  
   In your program, include the **LoaderLib**. This library can be easily modified to remove my specific logging implementation. Its primary purpose is to facilitate returning to the original menu, allowing users to choose a program to run again.

5. **Custom Implementation**  
   Feel free to implement your own version of the loading function, which may be more streamlined. The crucial requirement is to load **firmware.bin** from the **Programs/Main OS** folder into flash at the end of your program’s execution.