if status is-interactive
    export QT_QPA_PLATFORMTHEME=gtk3
    export PATH="$PATH:/home/kibertod/.dotnet/tools"
    export TERM="xterm-256color"
    clear;
    set -g fish_greeting
end

source /home/kibertod/dev/penis_bot/env.sh

function nvim
    kitty @ set-spacing padding=0 && /usr/bin/nvim $argv && kitty @ set-spacing padding=20
end

function nokitty-nvim
    /usr/bin/nvim $argv
end

function please
    sudo $argv
end

# Created by `pipx` on 2025-03-23 08:35:10
set PATH $PATH /home/kibertod/.local/bin
