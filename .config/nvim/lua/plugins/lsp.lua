---@diagnostic disable: duplicate-set-field
return {
	--lsp-config
	{
		"neovim/nvim-lspconfig",
		init_options = {
			userLanguages = {
				eelixir = "html-eex",
				eruby = "erb",
				rust = "html",
			},
		},
		init = function()
			local lspconfig = require("lspconfig")

			vim.keymap.set("n", "<leader>gD", vim.lsp.buf.declaration)
			vim.keymap.set("n", "<leader>gd", vim.lsp.buf.definition)
			vim.keymap.set("n", "<leader>gi", vim.lsp.buf.implementation)
			vim.keymap.set("n", "<leader>k", vim.lsp.buf.hover)

			-- nvim-cmp supports additional completion capabilities
			local capabilities = vim.lsp.protocol.make_client_capabilities()
			capabilities = require("cmp_nvim_lsp").default_capabilities(capabilities)

			lspconfig.rust_analyzer.setup({
				settings = {
					["rust-analyzer"] = {},
				},
				capabilities = capabilities,
			})
			lspconfig.emmet_language_server.setup({})
			lspconfig.htmx.setup({})
			lspconfig.pyright.setup({ capabilities = capabilities })
			lspconfig.csharp_ls.setup({ capabilities = capabilities })
			lspconfig.sqlls.setup({})
			lspconfig.cssls.setup({})
			lspconfig.clangd.setup({})

			local border = {
				{ "╭", "FloatBorder" },
				{ "─", "FloatBorder" },
				{ "╮", "FloatBorder" },
				{ "│", "FloatBorder" },
				{ "╯", "FloatBorder" },
				{ "─", "FloatBorder" },
				{ "╰", "FloatBorder" },
				{ "│", "FloatBorder" },
			}

			local orig_util_open_floating_preview = vim.lsp.util.open_floating_preview
			function vim.lsp.util.open_floating_preview(contents, syntax, opts, ...)
				opts = opts or {}
				opts.border = opts.border or border
				return orig_util_open_floating_preview(contents, syntax, opts, ...)
			end
		end,
	},
	--mason
	{
		"williamboman/mason.nvim",
		dependencies = {
			"neovim/nvim-lspconfig",
		},
		init = function()
			require("mason").setup()
			local signs = { Error = " ", Warn = " ", Hint = "󰌶 ", Info = " " }
			for type, icon in pairs(signs) do
				local hl = "DiagnosticSign" .. type
				vim.fn.sign_define(hl, { text = icon, texthl = hl, numhl = hl })
			end

			vim.diagnostic.config({
				virtual_text = { true, prefix = "●" },
				signs = true,
				underline = true,
				update_in_insert = false,
				severity_sort = true,
			})

			-- Add nvim-lspconfig plugin
			vim.keymap.set("n", "<leader>d", vim.diagnostic.open_float, {})

			vim.api.nvim_create_autocmd({ "CursorHold", "CursorHoldI" }, {
				group = vim.api.nvim_create_augroup("code_action_sign", { clear = true }),
				callback = function()
					require("code_action_utils").code_action_listener()
				end,
			})
		end,
	},
	--nvim-cmp
	{
		"hrsh7th/nvim-cmp",
		dependencies = {
			"hrsh7th/cmp-nvim-lsp",
			"saadparwaiz1/cmp_luasnip",
			"FelipeLema/cmp-async-path",
			"hrsh7th/cmp-nvim-lsp-signature-help",
			"L3MON4D3/LuaSnip",
			"onsails/lspkind.nvim",
		},
		init = function()
			local luasnip = require("luasnip")
			local cmp = require("cmp")
			cmp.setup({
				completion = {
					completeopt = "menu,menuone,noinsert",
				},
				snippet = {
					expand = function(args)
						luasnip.lsp_expand(args.body)
					end,
				},
				mapping = cmp.mapping.preset.insert({
					["<C-u>"] = cmp.mapping.scroll_docs(-4), -- Up
					["<C-d>"] = cmp.mapping.scroll_docs(4), -- Down
					["<Tab>"] = cmp.mapping.confirm({ select = true }),
				}),
				sources = {
					{ name = "nvim_lsp" },
					{ name = "luasnip" },
					{ name = "async_path" },
					-- { name = "nvim_lsp_signature_help" },
				},
				window = {
					completion = cmp.config.window.bordered({
						winhighlight = "Normal:Pmenu,FloatBorder:Pmenu,Search:None",
						col_offset = -3,
						side_padding = 0,
					}),
					documentation = cmp.config.window.bordered({
						winhighlight = "Normal:Pmenu,FloatBorder:Pmenu,Search:None",
						col_offset = -3,
						side_padding = 0,
					}),
				},
				formatting = {
					expandable_indicator = true,
					fields = { "kind", "abbr", "menu" },
					format = require("lspkind").cmp_format({
						mode = "symbol",
						maxwidth = 30,
						ellipsis_char = "...",
						show_labelDetails = true,
					}),
				},
			})
		end,
	},
	{
		{
			"folke/lazydev.nvim",
			ft = "lua",
			opts = {
				library = {
					{ path = "luvit-meta/library", words = { "vim%.uv" } },
				},
			},
		},
	},
	{
		"stevearc/conform.nvim",
		opts = {
			log_level = vim.log.levels.TRACE,
			formatters = {
				csharpier = {
					args = { "--write-stdout", "--no-cache", "$FILENAME" },
				},
			},
		},
		init = function()
			require("conform").setup({
				format_on_save = {
					lsp_format = "fallback",
					timeout_ms = 500,
				},
				formatters_by_ft = {
					lua = { "stylua" },
					python = { "black" },
					rust = { "rustfmt" },
					javascript = { "biome" },
					cpp = { "clang-format" },
					c = { "clang-format" },
					cs = { "clang-format" },
					css = { "prettierd" },
					scss = { "prettierd" },
					json = { "biome" },
					html = { "prettier" },
					toml = { "taplo" },
				},
				formatters = {
					rustfmt = {
						prepend_args = { "--config-path", "/home/kibertod/.config/rustfmt/rustfmt.toml" },
					},
				},
			})
			vim.keymap.set("n", "<leader>F", require("conform").format)
		end,
	},
	{
		"mfussenegger/nvim-lint",
	},
}
