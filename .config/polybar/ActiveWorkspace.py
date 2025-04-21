from i3ipc import Connection, Event

i3 = Connection()

focused = i3.get_tree().find_focused().workspace().num

out = ""

for i in range(1, 10):
    if i == focused:
        out += "%{F#66667f}"

    if i != 1:
        out += " "

    out += str(" ") + "%{F-}"

print(out)
