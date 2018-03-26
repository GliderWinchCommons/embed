# ~/.profile: executed by the command interpreter for login shells.
# This file is not read by bash(1), if ~/.bash_profile or ~/.bash_login
# exists.
# see /usr/share/doc/bash/examples/startup-files for examples.
# the files are located in the bash-doc package.

# the default umask is set in /etc/profile; for setting the umask
# for ssh logins, install and configure the libpam-umask package.
#umask 022

# if running bash
if [ -n "$BASH_VERSION" ]; then
    # include .bashrc if it exists
    if [ -f "$HOME/.bashrc" ]; then
	. "$HOME/.bashrc"
    fi
fi

# set PATH so it includes user's private bin if it exists
if [ -d "$HOME/bin" ] ; then
    PATH="$HOME/bin:$PATH"
fi

export FITIP=47.204.250.158
# ends 20170916: 47.204.234.231

# set PATH so it includes dateselect if it exists
if [ -d "$HOME/GliderWinchCommons/embed/svn_pod/sw_PC/trunk/dateselect" ] ; then
    PATH="$PATH:$HOME/GliderWinchCommons/embed/svn_pod/sw_PC/trunk/dateselect"
fi

if [ -d "$HOME/svn_pod/sw_PC/trunk/tunnel_scripts" ] ; then
    PATH="$PATH:$HOME/svn_pod/sw_PC/trunk/tunnel_scripts"
fi


if [ -x ~/.arm_tools ]; then
	. ~/.arm_tools
fi


if [ -d "/usr/local/lib" ] ; then
    PATH="$PATH:/usr/local/lib"
fi

export PATH=$PATH:/home/deh/jdk1.8.0_121/db/bin:/home/deh/jdk1.8.0_121/bin

