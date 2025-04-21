return {
	{
		"folke/flash.nvim",
		event = "VeryLazy",
		---@type Flash.Config
		opts = {},
        -- stylua: ignore
        keys = {
              { "s", mode = { "n", "x", "o" }, function() require("flash").jump() end, desc = "Flash" },
              { "S", mode = { "n", "x", "o" }, function() require("flash").treesitter() end, desc = "Flash Treesitter" },
              { "r", mode = "o", function() require("flash").remote() end, desc = "Remote Flash" },
              { "R", mode = { "o", "x" }, function() require("flash").treesitter_search() end, desc = "Treesitter Search" },
              { "<c-s>", mode = { "c" }, function() require("flash").toggle() end, desc = "Toggle Flash Search" },
        },
	},
	{
		"windwp/nvim-autopairs",
		event = "InsertEnter",
		config = true,
		init = function()
			require("nvim-autopairs").setup()
		end,
	},
	{
		"rmagatti/auto-session",
		lazy = false,

		---enables autocomplete for opts
		---@module "auto-session"
		---@type AutoSession.Config
		opts = {
			suppressed_dirs = { "~/", "~/Projects", "~/Downloads", "/" },
			-- log_level = 'debug',
		},
	},
	-- {
	-- 	"coffebar/neovim-project",
	-- 	opts = {
	-- 		projects = { -- define project roots
	-- 			"~/dev/*",
	-- 		},
	-- 		picker = {
	-- 			type = "telescope", -- or "fzf-lua"
	-- 			preview = true, -- show directory structure preview in Telescope
	-- 		},
	-- 	},
	-- 	init = function()
	-- 		vim.opt.sessionoptions:append("globals")
	-- 		vim.keymap.set("n", "<leader>pp", "<CMD>:NeovimProjectDiscover<CR>")
	-- 	end,
	-- 	dependencies = {
	-- 		{ "nvim-lua/plenary.nvim" },
	-- 		{ "nvim-telescope/telescope.nvim", tag = "0.1.4" },
	-- 		{ "ibhagwan/fzf-lua" },
	-- 		{ "Shatur/neovim-session-manager" },
	-- 	},
	-- 	lazy = false,
	-- 	priority = 100,
	-- },
	-- {
	-- 	"cameron-wags/splash.nvim",
	-- 	init = function()
	-- 		require("splash").setup({
	-- 			text = {
	-- 				"                      :::!~!!!!!:.",
	-- 				"                  .xUHWH!! !!?M88WHX:.",
	-- 				"                .X*#M@$!!  !X!M$$$$$$WWx:.",
	-- 				"               :!!!!!!?H! :!$!$$$$$$$$$$8X:",
	-- 				"              !!~  ~:~!! :~!$!#$$$$$$$$$$8X:",
	-- 				"             :!~::!H!<   ~.U$X!?R$$$$$$$$MM!",
	-- 				"             ~!~!!!!~~ .:XW$$$U!!?$$$$$$RMM!",
	-- 				'               !:~~~ .:!M"T#$$$$WX??#MRRMMM!',
	-- 				'               ~?WuxiW*`   `"#$$$$8!!!!??!!!',
	-- 				'             :X- M$$$$       `"T#$T~!8$WUXU~',
	-- 				"            :%`  ~#$$$m:        ~!~ ?$$$$$$",
	-- 				'          :!`.-   ~T$$$$8xx.  .xWW- ~""##*"',
	-- 				".....   -~~:<` !    ~?T#$$@@W@*?$$      /`",
	-- 				'W$@@M!!! .!~~ !!     .:XUW$W!~ `"~:    :',
	-- 				'#"~~`.:x%`!!  !H:   !WM$$$$Ti.: .!WUn+!`',
	-- 				':::~:!!`:X~ .: ?H.!u "$$$B$$$!W:U!T$$M~',
	-- 				'.~~   :X@!.-~   ?@WTWo("*$$$W$TH$! `',
	-- 				'Wi.~!X$?!-~    : ?$$$B$Wu("**$RM!       the skeleton says:',
	-- 				"$R@i.~~ !     :   ~$$$$$B$$en:``            Whatchu lookin at fool?",
	-- 				'?MXT@Wx.~    :     ~"##*$$$$M~              Write the damn code!',
	-- 			},
	-- 			vim_height = 20,
	-- 			vim_width = 40,
	-- 		})
	-- 	end,
	-- },
}
