return {
	{
		"nvim-tree/nvim-tree.lua",
		dependencies = {
			"nvim-tree/nvim-web-devicons",
		},
		init = function()
			require("nvim-tree").setup({
				filters = { dotfiles = false },
				git = { enable = true, ignore = false },
			})
			vim.keymap.set("n", "<leader>tt", "<CMD>:NvimTreeToggle<CR>")
		end,
	},
	{
		"nvim-lualine/lualine.nvim",
		dependencies = { "nvim-tree/nvim-web-devicons" },
		init = function()
			local lualine = require("lualine")
			local config = {
				options = {
					section_separators = { right = "", left = "" },
					component_separators = { right = "", left = "" },
				},
				sections = {
					lualine_a = {
						{
							require("noice").api.statusline.mode.get,
							cond = require("noice").api.statusline.mode.has,
						},
						"branch",
					},
					lualine_b = { "filename" },
					lualine_c = {
						"filetype",
						"encoding",
					},
					lualine_x = {
						{ "diagnostics", symbols = { error = " ", warn = " ", info = " " } },
					},
					lualine_y = {
						{
							"diff",
							symbols = { added = " ", modified = "󰝤 ", removed = " " },
						},
					},
					lualine_z = { "progress", "location" },
				},
			}
			lualine.setup(config)
		end,
	},
	{
		"folke/trouble.nvim",
		opts = {}, -- for default options, refer to the configuration section for custom setup.
		cmd = "Trouble",
		--stylua: ignore
		keys = {
			{ "<leader>xX", "<cmd>Trouble diagnostics toggle<cr>", desc = "Diagnostics (Trouble)", },
			{ "<leader>xx", "<cmd>Trouble diagnostics toggle filter.buf=0<cr>", desc = "Buffer Diagnostics (Trouble)",},
			{ "<leader>cs", "<cmd>Trouble symbols toggle focus=false<cr>", desc = "Symbols (Trouble)",},
			{ "<leader>cl", "<cmd>Trouble lsp toggle focus=false win.position=right<cr>", desc = "LSP Definitions / references / ... (Trouble)",},
			{ "<leader>xL", "<cmd>Trouble loclist toggle<cr>", desc = "Location List (Trouble)",},
			{ "<leader>xQ", "<cmd>Trouble qflist toggle<cr>", desc = "Quickfix List (Trouble)",},
        },
		--stylua
	},
	{
		"lukas-reineke/indent-blankline.nvim",
		opts = {},
		main = "ibl",
		---@module "ibl"
		---@type ibl.config
		init = function()
			require("ibl").setup({
				scope = {
					enabled = false,
				},
				indent = {
					char = "╎",
				},
				exclude = {
					filetypes = {
						"lspinfo",
						"packer",
						"checkhealth",
						"help",
						"man",
						"gitcommit",
						"TelescopePrompt",
						"TelescopeResults",
						"''",
						"dashboard",
					},
				},
			})
		end,
	},
	{
		"kdheepak/lazygit.nvim",
		dependencies = { "nvim-lua/plenary.nvim" },
		init = function()
			vim.keymap.set("n", "<leader>gg", "<cmd>LazyGit<CR>")
		end,
	},
	{
		"lewis6991/gitsigns.nvim",
		init = function()
			require("gitsigns").setup()
		end,
	},
	{
		"akinsho/toggleterm.nvim",
		version = "*",
		opts = {},
		init = function()
			vim.keymap.set("n", "<C-t>", "<cmd>ToggleTerm<CR>")
			vim.keymap.set("t", "<C-t>", "<cmd>ToggleTerm<CR>")
		end,
	},
	{
		"akinsho/bufferline.nvim",
		version = "*",
		dependencies = { "nvim-tree/nvim-web-devicons", "Asheq/close-buffers.vim" },
		init = function()
			require("bufferline").setup({
				options = {
					offsets = {
						{ filetype = "NvimTree", text = "File Explorer", highlight = "Directory", separator = true },
					},
				},
			})
			vim.keymap.set("n", "<leader>bd", "<CMD>Bdelete hidden<CR>")
			vim.keymap.set("n", ",", "<CMD>bp<CR>")
			vim.keymap.set("n", ".", "<CMD>bn<CR>")
		end,
	},
	{
		"stevearc/aerial.nvim",
		opts = {},
		dependencies = {
			"nvim-treesitter/nvim-treesitter",
			"nvim-tree/nvim-web-devicons",
		},
		init = function()
			require("aerial").setup({
                backends = {"treesitter"},
				on_attach = function(bufnr)
					vim.keymap.set("n", "K", "<cmd>AerialPrev<CR>", { buffer = bufnr })
					vim.keymap.set("n", "J", "<cmd>AerialNext<CR>", { buffer = bufnr })
				end,
			})
			-- You probably also want to set a keymap to toggle aerial
			vim.keymap.set("n", "<leader>a", "<cmd>AerialToggle!<CR>")
		end,
	},
	{
		"leath-dub/snipe.nvim",
        --stylua: ignore
		keys = {
			{
                "<leader>s", function() require("snipe").open_buffer_menu() end,
                desc = "Open Snipe buffer menu",
            },
		},
		init = function()
			require("snipe").setup({
				ui = {
					position = "center",
				},
			})
		end,
		-- stylua
		opts = {},
	},
	{
		"nvim-telescope/telescope.nvim",
		branch = "0.1.x",
		dependencies = {
			"nvim-lua/plenary.nvim",
			{
				"nvim-telescope/telescope-fzf-native.nvim",
				build = "cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release && "
					.. "cmake --build build --config Release && "
					.. "cmake --install build --prefix build",
			},
			"nvim-tree/nvim-web-devicons",
			"fannheyward/telescope-coc.nvim",
			"nvim-telescope/telescope-ui-select.nvim",
		},
		config = function()
			local telescope = require("telescope")
			local actions = require("telescope.actions")
			local builtin = require("telescope.builtin")
			local themes = require("telescope.themes")

			telescope.setup({
				extensions = {
					["ui-select"] = { themes.get_dropdown() },
				},
				defaults = {
					prompt_prefix = " ",
					selection_caret = "➜ ",
					mappingsd = {
						i = {
							["<C-k>"] = actions.move_selection_previous,
							["<C-j>"] = actions.move_selection_next,
							["<C-q>"] = actions.send_selected_to_qflist + actions.open_qflist,
						},
					},
				},
			})

			telescope.load_extension("ui-select")

			vim.keymap.set("n", "<leader>ff", builtin.find_files, {})
			vim.keymap.set("n", "<leader>fb", builtin.buffers, {})
			vim.keymap.set("n", "<leader>fg", builtin.live_grep, {})
			vim.keymap.set("n", "<leader>ft", builtin.treesitter, {})
			vim.keymap.set("n", "<leader>ca", vim.lsp.buf.code_action, {})
		end,
	},
	{
		"folke/noice.nvim",
		event = "VeryLazy",
		opts = {},
		dependencies = {
			"MunifTanjim/nui.nvim",
			"rcarriga/nvim-notify",
		},
		init = function()
			require("noice").setup({
				lsp = {
					override = {
						["vim.lsp.util.convert_input_to_markdown_lines"] = true,
						["vim.lsp.util.stylize_markdown"] = true,
						["cmp.entry.get_documentation"] = true,
					},
				},
				presets = {
					bottom_search = true,
					command_palette = true,
					long_message_to_split = true,
					inc_rename = false,
					lsp_doc_border = false,
				},
			})
		end,
	},
	{
		"folke/todo-comments.nvim",
		dependencies = { "nvim-lua/plenary.nvim" },
		opts = {},
		init = function()
			vim.keymap.set("n", "<leader>td", "<CMD>TodoTelescope<CR>")
		end,
	},
}
