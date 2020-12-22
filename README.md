# FileRemover
This project was designed to help comprehend the plethora of components, I've learnt from COMP1521.
# File Remover (Take precautions if necessary)!

## Normal File Remover:

1. Remove Files by a specified name by a user (Recursive Search).
   - Defined by the argument **-n**.
   - If a file relative to current directory remove them.
   - Add Optional File Exists or doesn't exist.
   - Example: **./file_remover -n "test.txt"** removes files named test.txt.

## Prefix File Remover:

2. Remove Files/Folders by a specified prefix by a user (Recursive Search).
   - Defined by the argument **-p**
   - If a file/folder relative to current directory remove them with given prefix.
   - Example: **./file_remover -p "test"** removes all the files with prefix test.

## Old File Remover:

3. Remove File/Folders by a specified :watch: (based on c_time in the struct stat) by a user (Recursive Search).
   - Defined by the argument **-t**
   - Find files/folders that are relative to current directory and remove any of them if the time created is older than that period (old_timestamp <= time_specified).
   - A timestamp is defined as Day/Month/Year Hour:Minute:Second.
   - Example: **./file_remover -t "12/09/2020 10:30:00"**

## Grep File Remover:

4. Remove File that contain specific text in that file (Recursive Search).

   - Defined by the argument **-g**.

   - Inspired by the **grep** command in Linux. 

     [More here](https://www.howtogeek.com/496056/how-to-use-the-grep-command-on-linux/)

   - Two parts to this section, you can either remove files based on one search term or multiple.

     > **./file_remover -g "Hello World" ** -> Remove files containing Hello World
     >
     > **./file_remover -g "Hello World" "Goodbye World"** -> Remove files containing Hello World or Goodbye World
     >
     > We'll be using **grep -E -w -l "text" *.c** command to do this.

