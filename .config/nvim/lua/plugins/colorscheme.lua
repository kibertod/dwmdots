return {
	"vague2k/huez.nvim",
	dependencies = {
		"norcalli/nvim-colorizer.lua",
		"slugbyte/lackluster.nvim",
		"rebelot/kanagawa.nvim",
		"vague2k/vague.nvim",
		"catppuccin/nvim",
		"miyakogi/seiya.vim",
	},
	config = function()
		require("huez").setup()
		require("colorizer").setup()

		local colors = {
			gray = "#969896",
			fg = "#C5C9C5",
			bg = "#181616",
			bg_selected = "#1A1A1A",
			purple = "#938AA9",
			red = "#C4746E",
			green = "#8A9A7B",
			orange = "#B98D7B",
			yellow = "#E6C384",
			blue = "#8BA4B0",
			cyan = "#8EA4A2",
			black = "#0D0C0C",
		}

		vim.api.nvim_set_hl(0, "Normal", { bg = bg })

		-- split
		vim.api.nvim_set_hl(0, "WinSeparator", { bg = colors.bg_selected, fg = colors.bg_selected })

		-- telescope
		vim.api.nvim_set_hl(0, "TelescopeBorder", { fg = colors.fg })
		vim.api.nvim_set_hl(0, "TelescopePromptNormal", { fg = colors.fg, bg = colors.bg_selected })
		vim.api.nvim_set_hl(0, "TelescopePromptTitle", { fg = colors.bg, bg = colors.blue })
		vim.api.nvim_set_hl(0, "TelescopeResultsNormal", { fg = colors.fg, bg = colors.bg_selected })
		vim.api.nvim_set_hl(0, "TelescopeResultsTitle", { fg = colors.bg, bg = colors.purple })
		vim.api.nvim_set_hl(0, "TelescopePreviewNormal", { fg = colors.fg, bg = colors.bg_selected })
		vim.api.nvim_set_hl(0, "TelescopePreviewTitle", { fg = colors.bg, bg = colors.yellow })

		-- nvim-tree
		vim.api.nvim_set_hl(0, "NvimTreeNormal", { bg = colors.bg_selected })
		vim.api.nvim_set_hl(0, "NvimTreeNormalNC", { bg = colors.bg_selected })

		-- lsp hovers
		vim.api.nvim_set_hl(0, "NormalFloat", { bg = colors.bg_selected })
		vim.api.nvim_set_hl(0, "FloatBorder", { bg = colors.bg_selected, fg = colors.fg })

		-- Customization for Pmenu
		vim.api.nvim_set_hl(0, "PmenuSel", { bg = colors.fg, fg = colors.bg })
		vim.api.nvim_set_hl(0, "Pmenu", { fg = colors.fg, bg = colors.bg_selected })

		vim.api.nvim_set_hl(0, "CmpItemAbbrDeprecatedIcon", { fg = colors.fg, bg = "NONE", strikethrough = true })
		vim.api.nvim_set_hl(0, "CmpItemAbbrMatchIconIcon", { fg = colors.blue, bg = "NONE", bold = true })
		vim.api.nvim_set_hl(0, "CmpItemAbbrMatchFuzzyIcon", { fg = colors.blue, bg = "NONE", bold = true })
		vim.api.nvim_set_hl(0, "CmpItemMenuIcon", { fg = colors.fg, bg = "NONE", italic = true })

		vim.api.nvim_set_hl(0, "CmpItemKindFieldIcon", { bg = colors.gray, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindPropertyIcon", { bg = colors.gray, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindEventIcon", { bg = colors.gray, fg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemKindTextIcon", { bg = colors.blue, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindKeywordIcon", { bg = colors.blue, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindValueIcon", { bg = colors.blue, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindEnumMemberIcon", { bg = colors.blue, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindTypeParameterIcon", { bg = colors.blue, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindColorIcon", { bg = colors.blue, fg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemKindConstantIcon", { bg = colors.purple, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindConstructorIcon", { bg = colors.purple, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindReferenceIcon", { bg = colors.purple, fg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemKindStructIcon", { bg = colors.orange, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindClassIcon", { bg = colors.orange, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindEnumIcon", { bg = colors.orange, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindInterfaceIcon", { bg = colors.orange, fg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemKindFunctionIcon", { bg = colors.red, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindOperatorIcon", { bg = colors.red, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindMethodIcon", { bg = colors.red, fg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemKindModuleIcon", { bg = colors.green, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindVariableIcon", { bg = colors.green, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindFileIcon", { bg = colors.green, fg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemKindUnitIcon", { bg = colors.yellow, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindSnippetIcon", { bg = colors.yellow, fg = colors.bg })
		vim.api.nvim_set_hl(0, "CmpItemKindFolderIcon", { bg = colors.yellow, fg = colors.bg })
	end,
}
