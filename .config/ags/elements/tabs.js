export function HeaderWithIcon(label, icon) {
	return Widget.Box({
		spacing: 10,
		children: [
			Widget.Label({
				hexpand: true,
				label: label,
				xalign: 0,
				class_name: "tabs-header-label",
			}),
			Widget.Label({ label: icon, xalign: 1, class_name: "nerd" }),
		],
	});
}

export function Tabs(children) {
	let res = Widget.Box({
		attribute: 0,
		hexpand: true,
		vertical: true,
		spacing: 20,
		children: [],
	});

	let buttons = [];
	let revealers = [];

	for (let i = 0; i < children.length; i++) {
		let header = children[i].header;
		let content = children[i].content;

		buttons.push(
			Widget.Button({
				class_name: "tabs-header",
				child: header,
				onClicked: (self) => {
					let active_tab = self.parent.parent.attribute;
					self.parent.parent.attribute = i;

					self.parent.children[active_tab].class_name = "tabs-header";
					self.class_name = "tabs-header active";

					self.parent.parent.children[1].children[active_tab].revealChild =
						false;
					self.parent.parent.children[1].children[i].revealChild = true;
				},
			}),
		);

		revealers.push(
			Widget.Revealer({
				revealChild: false,
				transition: "slide_down",
				transitionDuration: 0,
				child: Widget.Scrollable({ css: "min-height: 200px", child: content }),
			}),
		);
	}

	buttons[0].class_name = "tabs-header active";
	revealers[0].revealChild = true;

	res.children = [
		Widget.Box({ hpack: "fill", children: buttons }),
		Widget.Box({
			class_name: "tabs-content",
			children: revealers,
			vertical: true,
		}),
	];
	return res;
}
