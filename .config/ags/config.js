import { Network } from "./widgets/network.js";
import { Bluetooth } from "./widgets/bluetooth.js";
import { Powerprofile } from "./widgets/powerprofile.js";
import { Powermenu } from "./widgets/powermenu.js";
import { Clock } from "./widgets/clock.js";
import { Audio } from "./widgets/audio.js";
import { Mpd } from "./widgets/mpd.js";
import { Tabs, HeaderWithIcon } from "./elements/tabs.js";

const sidebar = Widget.Window({
	visible: false,
	monitor: 1,
	name: "sidebar",
	anchor: ["top", "left", "bottom"],
	exclusivity: "default",
	child: Widget.Box({
		css: "padding: 20px",
		spacing: 20,
		children: [
			Widget.Box({ css: "padding: 0 10px;", children: [Powermenu()] }),
			Widget.Separator(),
			Mpd(),
			Widget.Separator(),
			Tabs([
				{ header: HeaderWithIcon("Network", "󰖩"), content: Network() },
				{ header: HeaderWithIcon("Bluetooth", ""), content: Bluetooth() },
				{ header: HeaderWithIcon("Audio", "󰕾"), content: Audio() },
			]),
			Widget.Separator(),
			Powerprofile(),
			Widget.Separator(),
			Clock(),
		],
		vpack: "fill",
		vertical: true,
	}),
});

const scss = `${App.configDir}/style.scss`;
const css = `/tmp/my-style.css`;
Utils.exec(`sassc ${scss} ${css}`);

App.config({
	style: css,
	windows: [sidebar],
});

Utils.monitorFile(
	`${App.configDir}/style.scss`,

	function () {
		const scss = `${App.configDir}/style.scss`;

		const css = `/tmp/my-style.css`;

		Utils.exec(`sassc ${scss} ${css}`);
		App.resetCss();
		App.applyCss(css);
	},
);
