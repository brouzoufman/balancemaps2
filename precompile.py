from __future__ import print_function

VERSIONTEMPLATE = """\
// This file is automatically generated when building the PK3 file. Don't edit
//  it manually - look in precompile.py instead.

#define VERSION_MAJOR   {major}
#define VERSION_MINOR   {minor}
#define VERSION_BUGFIX  {bugfix}

#define VERSION_STRING       "{major}.{minor}.{bugfix}"
#define VERSION_COMMIT       "{commit}"
#define VERSION_COMMIT_LONG  "{commit_long}"
"""

KEYCONFTEMPLATE = """\
// This file is automatically generated when building the PK3 file. Don't edit
//  it manually - look in precompile.py instead.

alias bmaps_version "echo \\\"Balancemaps 2, \\civ{major}.{minor}.{bugfix}\\c-, commit \\cd{commit}\\c-\\\""
"""

VERSION_MAJOR   = 0
VERSION_MINOR   = 6
VERSION_BUGFIX  = 0

GIT_BRANCH      = open(".git/HEAD").read()[len("ref: refs/heads/"):].strip()
GIT_COMMIT_LONG = open(".git/refs/heads/" + GIT_BRANCH).read().strip()
GIT_COMMIT      = GIT_COMMIT_LONG[:8]

def precompile(*whoCares):
    print("current branch:", GIT_BRANCH)
    print("current commit:", GIT_COMMIT_LONG)
    
    vdict = {
        "major":  VERSION_MAJOR,
        "minor":  VERSION_MINOR,
        "bugfix": VERSION_BUGFIX,
        
        "commit":      GIT_COMMIT,
        "commit_long": GIT_COMMIT_LONG,
    }
    
    versionFile = open("pk3/acs/version.h", "w")
    versionFile.write(VERSIONTEMPLATE.format(**vdict))
    versionFile.close()
    
    keyconfFile = open("pk3/KEYCONF.version", "w")
    keyconfFile.write(KEYCONFTEMPLATE.format(**vdict))
    versionFile.close()