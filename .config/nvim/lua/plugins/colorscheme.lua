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

		vim.api.nvim_set_hl(0, "Normal", { bg = "#0f0f0f" })

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
		vim.api.nvim_set_hl(0, "Pmenu", { fg = colors.fg, bg = colors.bg })

		vim.api.nvim_set_hl(0, "CmpItemAbbrDeprecated", { fg = colors.fg, bg = "NONE", strikethrough = true })
		vim.api.nvim_set_hl(0, "CmpItemAbbrMatch", { fg = colors.blue, bg = "NONE", bold = true })
		vim.api.nvim_set_hl(0, "CmpItemAbbrMatchFuzzy", { fg = colors.blue, bg = "NONE", bold = true })
		vim.api.nvim_set_hl(0, "CmpItemMenu", { fg = colors.fg, bg = "NONE", italic = true })

		vim.api.nvim_set_hl(0, "CmpItemKindField", { bg = colors.bg, fg = colors.gray })
		vim.api.nvim_set_hl(0, "CmpItemKindProperty", { bg = colors.bg, fg = colors.gray })
		vim.api.nvim_set_hl(0, "CmpItemKindEvent", { bg = colors.bg, fg = colors.gray })

		vim.api.nvim_set_hl(0, "CmpItemKindText", { bg = colors.bg, fg = colors.blue })
		vim.api.nvim_set_hl(0, "CmpItemKindKeyword", { bg = colors.bg, fg = colors.blue })
		vim.api.nvim_set_hl(0, "CmpItemKindValue", { bg = colors.bg, fg = colors.blue })
		vim.api.nvim_set_hl(0, "CmpItemKindEnumMember", { bg = colors.bg, fg = colors.blue })
		vim.api.nvim_set_hl(0, "CmpItemKindTypeParameter", { bg = colors.bg, fg = colors.blue })
		vim.api.nvim_set_hl(0, "CmpItemKindColor", { bg = colors.bg, fg = colors.blue })

		vim.api.nvim_set_hl(0, "CmpItemKindConstant", { bg = colors.bg, fg = colors.purple })
		vim.api.nvim_set_hl(0, "CmpItemKindConstructor", { bg = colors.bg, fg = colors.purple })
		vim.api.nvim_set_hl(0, "CmpItemKindReference", { bg = colors.bg, fg = colors.purple })

		vim.api.nvim_set_hl(0, "CmpItemKindStruct", { bg = colors.bg, fg = colors.orange })
		vim.api.nvim_set_hl(0, "CmpItemKindClass", { bg = colors.bg, fg = colors.orange })
		vim.api.nvim_set_hl(0, "CmpItemKindEnum", { bg = colors.bg, fg = colors.orange })
		vim.api.nvim_set_hl(0, "CmpItemKindInterface", { bg = colors.bg, fg = colors.orange })

		vim.api.nvim_set_hl(0, "CmpItemKindFunction", { bg = colors.bg, fg = colors.red })
		vim.api.nvim_set_hl(0, "CmpItemKindOperator", { bg = colors.bg, fg = colors.red })
		vim.api.nvim_set_hl(0, "CmpItemKindMethod", { bg = colors.bg, fg = colors.red })

		vim.api.nvim_set_hl(0, "CmpItemKindModule", { bg = colors.bg, fg = colors.green })
		vim.api.nvim_set_hl(0, "CmpItemKindVariable", { bg = colors.bg, fg = colors.green })
		vim.api.nvim_set_hl(0, "CmpItemKindFile", { bg = colors.bg, fg = colors.green })

		vim.api.nvim_set_hl(0, "CmpItemKindUnit", { bg = colors.bg, fg = colors.yellow })
		vim.api.nvim_set_hl(0, "CmpItemKindSnippet", { bg = colors.bg, fg = colors.yellow })
		vim.api.nvim_set_hl(0, "CmpItemKindFolder", { bg = colors.bg, fg = colors.yellow })
	end,
}
