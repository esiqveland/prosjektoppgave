#!/usr/bin/python2

import sys
import socket
import os
import subprocess

prosjektdir = "/home/king/src/prosjektoppgave"
hostname = socket.gethostname()

os.chdir(prosjektdir)

def update_git():
    subprocess.call(["git", "pull"])
    subprocess.call(["make"])

def launch_load_distr():  
    subprocess.call(["bin/load_distributor_bin"])
    pass

def launch_search_engine():
    subprocess.call(["bin/search_bin"])
    pass

while True:
    update_git()

    if hostname[len(hostname)-1] == '0':
        launch_load_distr()
    else:
        launch_search_engine()

