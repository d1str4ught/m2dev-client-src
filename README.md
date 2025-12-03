# Client Source Repository

This repository contains the source code necessary to compile the game client executable.

# THE FOLLOWING INSTRUCTIONS, EXCLUDING "⚙️ How to Build" SECTION, AFFECT ONLY THIS BRANCH, INSIDE THIS REPO

## 🚀 Quick Setup & Build
This project uses Git Submodules to manage its dependencies (like LZO 2.10, mio, etc.). This ensures all library source code is fetched and handled by Git, guaranteeing you always have the correct version.

### 📥 Initial Installation
The single most important step before building is running the depinit.py script. This script automatically checks for, installs, copies headers, and cleans up all external dependencies.

#### 1. Clone the repository
```
git clone --recursive --branch exp_1/src-improvs-and-submodules --single-branch https://github.com/MindRapist/m2dev-client-src.git
cd m2dev-client-src
```
#### 2. Run the Dependency Initialization script (⚠️⚠️⚠️ MUST be done at least once before first build!!! ⚠️⚠️⚠️)
This step automatically fetches all submodules, copies required headers, and cleans up temporary source files.
```
python3 depinit.py
```

## ⚙️ How to Build
After the initial clone, the project can be built with the standard CMake two-step process.
Please make sure that you have ran depinit.py, ```git submodule sync``` and ```git submodule update --init --recursive``` at least once before running the following commands.

#### 1. Configure the project (Triggers automated dependency checks and fetching)
```
cmake -S . -B build
```

#### 4. Build the project
```
cmake --build build
```

##📦 Managing Dependencies (Advanced)
If you need to update or change the version of any library, use the git submodule commands directly. Submodules track external repositories right down to a specific commit hash, giving you precise control.

### Updating All Dependencies
To update all installed submodules to the absolute latest version available in their respective upstream repositories:

#### Updates all submodules to the latest commit on their tracking branch
```
git submodule update --remote
```

### Rolling Back or Switching Versions
To manually switch a single dependency to a specific tag or commit (e.g., rolling back Crypto++ to tag v8.6.0):

#### 1. Switch the submodule pointer to the target version (Requires the submodule to already be initialized)
```
git -C vendor/cryptopp/src checkout v8.6.0
```

#### 2. Stage the version change
```
git add vendor/cryptopp/src
```

#### 3. Commit the change to "save" it
```
git commit -m "Upgrade Cryptopp to v8.6.0"
```

#### 4. Delete the build folder and re-run CMake configuration
```
cmake -S . -B build
```
