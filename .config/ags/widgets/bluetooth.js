import { Dropdown } from "../elements/dropdown.js";

const bluetooth = await Service.import("bluetooth");

//const discoverable = Variable(false, {
//	poll: [
//		3000,
//		`bash -c "bluetoothctl show | grep Discoverable: | awk '{print $2}'"`,
//		(out) => (out == "yes" ? true : false),
//	],
//});
//
//const pairable = Variable(false, {
//	poll: [
//		3000,
//		`bash -c "bluetoothctl show | grep Pairable: | awk '{print $2}'"`,
//		(out) => (out == "yes" ? true : false),
//	],
//});

const toggleScan = Widget.Button({
	attribute: undefined,
	child: Widget.Label({ class_name: "nerd", label: "󱉶" }),
	name: "bluetooth-scan",
	class_name: "",
	css: "margin-bottom: 0",
	onClicked: (self) => {
		if (self.attribute == undefined) {
			self.attribute = Utils.subprocess(
				["python", ".config/ags/bluetooth/scan_on.py"],
				(out) => print(out),
				(err) => print(err),
				self,
			);
			self.class_name = "active";
		} else {
			self.attribute.force_exit();
			self.attribute = undefined;
			self.class_name = "";
		}
	},
});

//const toggleDiscoverable = Widget.Button({
//	child: Widget.Label({
//		label: discoverable.bind().as((d) => (d ? "" : "󰠥")),
//	}),
//	class_name: discoverable.bind().as((d) => (d ? "active" : "")),
//	name: "bluetooth-discoverable",
//	css: "margin-bottom: 0",
//	onClicked: () => {
//		if (discoverable.value) {
//			Utils.execAsync("bluetoothctl discoverable off");
//		} else {
//			Utils.execAsync("bluetoothctl discoverable on");
//		}
//	},
//});
//
//const togglePairable = Widget.Button({
//	child: Widget.Label({
//		label: "",
//	}),
//	class_name: pairable.bind().as((d) => (d ? "active" : "")),
//	name: "bluetooth-pairable",
//	css: "margin-bottom: 0",
//	onClicked: () => {
//		if (pairable.value) {
//			Utils.execAsync("bluetoothctl pairable off");
//		} else {
//			Utils.execAsync("bluetoothctl pairable on");
//		}
//	},
//});

const buttons = Widget.Box({
	vertical: true,
	spacing: 10,
	children: [
		Widget.Label("Actions"),
		toggleScan,
		//toggleDiscoverable,
		//togglePairable,
	],
});

const devices = Widget.Box({
	vertical: true,
	spacing: 10,
	hexpand: true,
	children: [
		Widget.Label("Devices"),
		Widget.Scrollable({
			css: "min-height: 150px;",
			class_name: "wifi-networks",
			child: Widget.Box({
				vertical: true,
				class_name: "bluetooth-devices",
				children: bluetooth.bind("devices").transform((devices) =>
					devices
						.filter((d) => d.name !== null)
						.map((device) =>
							Widget.Button({
								on_clicked: () => {
									device.setConnection(!device.connected);
								},
								child: Widget.Box({
									spacing: 8,
									children: [
										Widget.Label({
											hexpand: true,
											label: device.name,
											justification: "left",
											xalign: 0,
										}),
										Widget.Label({
											class_name: "nerd",
											label: device.bind("paired").as((p) => `${p ? "" : ""}`),
										}),
									],
								}),
								class_name: device
									.bind("connected")
									.as((c) => (c ? "active" : "")),
							}),
						),
				),
			}),
		}),
	],
});

export function Bluetooth() {
	return Widget.Box({
		class_name: "bluetooth",
		css: "min-height: 2px; min-width: 2px;",
		visible: true,
		spacing: 20,
		children: [devices, buttons],
	});
}
