
## Notes
Dmitri here; theres a lot here with a fairly complicated and "developer oriented" workflow.

I recommend going to "Setting up everything" to install what everyone has, followed by "Task list" to get an idea of what should be done, then "Contributing" and "Branches / Commits" to learn how to share your code.

Don't be afraid to ask on teams or raise a github issue on things that are just not working out for you or if the workflow is overwhelming. This is a bit of an experiment and I'll be more than happy to teach others what I know, even if we eventually want switch back to edstem.


## Contributing
Everything is written in "modules", with the exception of main.c and constants.h

Modules can be imported by others and can share structs and functions. As of writing, there exists group, solver, student, and utils, as examples to use.

This is how we may setup a simple "hello world" module:

1. Think of a simple yet informative name, eg: "HelloWorld"
2. Create a branch with the name "HelloWorld" (so no one interrupts your work)
3. Create a folder called "HelloWorld", with two files: "HelloWorld.c" and "HelloWorld.h". The c file will contain the functions that you want to write, and the header file with the functions that are shared.
4. Inside the "c" file, write the code you want to contribute, eg:
```
#include <stdlib.h>
#include <stdio.h>

#include "HelloWorld.h" // Always include your own module's header file so gcc can process it

/* Insert documentation here */
void hello(void) {
  printf("Hello world!");
}
``` 

5. Inside the header file, write:
```
#ifndef HELLOWORLD_H // <- Make sure this is your name followed by "_H"
#define HELLOWORLD_H

// Custom modules to include, eg such as constants or students:
#include "../constants.h"
#include "../student/student.h"

/* Your header functions (things you want to share) go here */
/* Note how the function is "skeletonised". It should only be fully written in HelloWorld.c */
void hello(void);

#endif
```

6. Edit the Makefile to include your module
```
# Place your module at the end separated with a space
SRCS = main.c *other modules* HelloWorld/HelloWorld.c
```

7. And now, we can edit the main.c file to finally use it!
```
#include "HelloWorld/HelloWorld.h" // Include the module

int main(void) {
  hello(); // The newly created function from HelloWorld.c!
}

```

### General tips

 - Document, document & document. Add comments to everything and use clear variable/function names. 
 - Minimize pull requests (finish your branch as much as you can before pushing to main).
 - Don't be afraid to create [issues](https://github.com/Aveygo/UniUnity/issues) to get some help.

## Setting up everything

This is for first time setup on windows. Most applies to linux but not anything with powershell or choco (just use apt). 

You can also completely skip all these steps by using github codespaces: Go to the [repo](https://github.com/Aveygo/UniUnity), Hit the "<> Code" button, codespaces, create codepsace. **NOTE**, we only get 60 hours per month and so should be preserved only for the most dire of circumstances. 

### Powershell
To install and run some things, we first open powershell with administrative permissions.
1. Hit the windows key and search for **powershell**
2. DO NOT open it just yet, rather, right click it and select: "Run as administrator"
3. After you accept the admin request, a blue terminal should appear

### Github keys
To authenticate with github, we need to share a "secret" with them.
This secret can be generate by typing in the powershell:
```
ssh-keygen -t ed25519
```
Press enter for all options until the program is done.

Now we need to actually show the secret that was generated. Look back at the logs for a line that says:

"Your public key has been saved in ___"

Copy whatever is in the blank area, it should look something like "c:/Users/username/.ssh/id_rsa.pub". It should contain a ".pub" at the end.

Next, type in the command:
```
type FILEPATH
```
Which should show the contents of the file.
Copy the contents to your clipboard and go to github -> click on your profile (top right) -> settings -> ssh and gpg keys -> new ssh key -> enter a name and paste the key into the larger field & save.


### Make
You will need to install **make**, which automates creating the gcc command for us.

Installing **make** is a bit of a hassle on windows, so we install [**choco**](https://chocolatey.org/install) first which will handle the installation for us. 
Run the following in the powershell instance (don't worry, its not a virus, check the choco link, it's the official way):
```
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```
followed by:
```
choco install make
```

### Git
Git is used as a version manager.
We can use choco again in the same elevated powershell instance:
```
choco install git.install
```
We also need to do a little setup with:
```
git config --global user.name "John Doe"
```
and
```
git config --global user.email johndoe@example.com
```
It is not a "critical" step but is required for git to work.


### VScode
Install and open [vscode](https://code.visualstudio.com/download).
This is where the code will be edited from.

NOTE: You will likely have popups for extensions, you can install all of them as they are very good and will likely help you out.

This is how we load the repository:
1. Look at the very bottom left for a blue button that kinda looks like "><".
2. A popup should appear, click remote repository (not github codespaces!), and likely sign into github.
3. Go back to vscode and click the button again, remote repository, and UniUnity should be available. Vscode will download the repository, but only in memory.
4. To locally edit the repo, click the button again, and select continue working on -> local clone, and select a directory for the files to be downloaded to.

You may see some scary/unknown popups that are git related. You should be fine accepting them all. They are mainly there for people who don't like stuff happening without them knowing. 

### Compile and run the script
You should now have the tools to run the main file.
Open a new terminal (ctrl+shift+backtick OR from the top toolbar "Terminal" -> "New Terminal") and type:
```
make
```
then
```
./main
```

If "command make is missing", see Setting up everything -> Make

If you see "make file missing/invalid", then you likely still in "remote development mode" and need to repeat step 4 of VScode.

If you made no changes to the code, then you might see "Nothing to be done for 'all'". If you did make changes however, then it's likely that you created an invalid module / didn't import it into main. See Contributing.

## Branches / Commits
We have done all this work to get to here.

When you are making changes to code, you should be doing them in your own "copy" of the code. </br>
This is known as a "branch". The "main" branch will be the final result when all is said and done.

So, lets say you want to add some cool functions to the current code.

1. Create a branch in vscode by clicking on the "check out" button next to the blue icon at the bottom right (it says the name of the current branch, eg: main), then typing in or selecting the name of the branch you want to continue on. Please note that everyone's branches are here so try and only select yours.

2. Click yes on anything that comes up (should be fine).

3. Make all the changes you want to your hearts content.

4. As you add, modify, or delete files, you'll see the "Source Control" tab (ctrl+shift+g) update it's pending changes.

5. If you want to upload your changes to the branch online, click on this tab and make sure to type in a short message to describe to others what you did. Hit Commit, Sync. And done! Repeat for any future changes on your branch.

6. Once your branch is finalized, create a request to merge with main by going to the "Github" tab on the right (should be a circular icon with a cat), then at the top select the "Create Pull Request" button, and create. This creates an offer to merge with main, it doesn't actually do the merge.

7. More often than not we'll have merge conflicts, where there are two pieces of code that interfere with each other. For now Ill (dmitri) will handle them. Once they are sorted, everyone should at a look at it and if everyone's happy, we merge!

## Debugging
Run the application with the "-d" argument, eg:
```
main -d
```