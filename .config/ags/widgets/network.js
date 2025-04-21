const { wifi, toggleWifi } = await Service.import("network");

const vpnStatus = Variable(false, {
	poll: [
		1000,
		`bash -c 'nmcli -t -f name connection show --active | grep -q "wg_private" && echo true || echo false'`,
		(out) => out == "true",
	],
});

const Vpn = Widget.Button({
	name: "vpn",
	child: Widget.Label("VPN"),
	css: "margin-bottom: 0",
	class_name: "",
	onClicked: (self) => {
		if (self.class_name === "active") {
			Utils.execAsync("nmcli connection down wg_private");
			Utils.execAsync(`nmcli connection up ${wifi.ssid}`);
		} else {
			Utils.execAsync("nmcli connection up wg_private");
			Utils.execAsync(`nmcli connection up ${wifi.ssid}`);
		}
	},
	setup: (self) =>
		self.hook(vpnStatus, (self) => {
			let class_name = vpnStatus.value ? "active" : "";
			if (self.class_name != class_name) {
				self.class_name = class_name;
			}
		}),
});

const wifiToggle = Widget.Button({
	name: "wifi-toggle",
	child: Widget.Label({ class_name: "nerd", label: "󰖪" }),
	class_name: wifi.bind("enabled").as((e) => (!e ? "active" : "")),
	onClicked: toggleWifi,
});

const Actions = Widget.Box({
	vertical: true,
	spacing: 10,
	children: [Widget.Label("Actions"), Vpn, wifiToggle],
});

function wifiAccessPoint(ap) {
	return Widget.Button({
		on_clicked: () => {
			Utils.execAsync(`nmcli device wifi connect ${ap.ssid}`);
		},
		child: Widget.Box({
			class_name: ap.ssid == wifi.ssid ? "wifi-active" : "",
			spacing: 8,
			children: [
				Widget.Label({
					hexpand: true,
					xalign: 0,
					label: ap.ssid || "",
					justification: "left",
				}),
				Widget.Label({
					class_name: "nerd",
					label: `${ap.strength > 80 ? "󰤨" : ap.strength > 65 ? "󰤥" : ap.strength > 50 ? "󰤢" : ap.strength > 35 ? "󰤠" : "󰤫"}`,
					justification: "right",
				}),
			],
		}),
	});
}

function getWifiAccessPoints() {
	if (!wifi.enabled)
		return [
			Widget.Label({ label: "Wifi is disabled", vexpand: true, vpack: "fill" }),
		];
	if (wifi.access_points.length <= 1) return [Widget.Spinner()];

	let res = [];
	let counter = 0;
	let names = [];
	let points = wifi.access_points.sort((a, b) => b.strength - a.strength);
	for (let i = 0; i < points.length; i++) {
		let ap = points[i];
		if (counter == 10) break;
		if (names.includes(ap.ssid)) continue;
		counter++;
		names.push(ap.ssid);
		if (ap.ssid == wifi.ssid) res = [wifiAccessPoint(ap), ...res];
		else res.push(wifiAccessPoint(ap));
	}
	return res;
}

const Networks = Widget.Box({
	vertical: true,
	hexpand: true,
	spacing: 10,
	children: [
		Widget.Label("Networks"),
		Widget.Scrollable({
			css: "min-height: 150px;",
			class_name: "wifi-networks",
			child: Widget.Box({
				vertical: true,
				children: getWifiAccessPoints(),
				setup: (self) =>
					self.hook(wifi, (self) => (self.children = getWifiAccessPoints())),
			}),
		}),
	],
});

export function Network() {
	let widget = Widget.Box({
		class_name: "network",
		hexpand: true,
		visible: true,
		spacing: 20,
		children: [Networks, Actions],
	});

	return widget;
}
