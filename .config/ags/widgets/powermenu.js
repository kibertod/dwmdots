function entry(icon, name, command) {
	return Widget.Button({
		child: Widget.Label({ class_name: "nerd", label: icon }),
		name: name,
		onClicked: () => {
			Utils.execAsync(command);
		},
	});
}

export function Powermenu() {
	let avatar = Widget.Box({
		class_name: "img ava",
		css: "background-image: url('/home/kibertod/.config/ags/ava.jpg')",
	});
	avatar = Widget.Box({
		vertical: true,
		hpack: "start",
		children: [avatar],
	});
	let powermenu = Widget.Box({
		hexpand: true,
		vertical: true,
		spacing: 8,
		children: [
			Widget.Label({
				class_name: "greeting",
				label: `Hello, ${Utils.exec("whoami")}!`,
			}),
			Widget.Box({
				homogeneous: true,
				spacing: 8,
				children: [
					entry("", "lock", "hyprlock"),
					entry("󰗽", "logout", "hyprctl dispatch exit"),
				],
			}),
			Widget.Box({
				homogeneous: true,
				spacing: 8,
				children: [
					entry("", "reboot", "systemctl reboot"),
					entry("󰤆", "poweroff", "systemctl poweroff"),
				],
			}),
		],
	});

	return Widget.Box({
		hexpand: true,
		spacing: 40,
		children: [powermenu, avatar],
	});
}
