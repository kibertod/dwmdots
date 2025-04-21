return {
    "neoclide/coc.nvim",
    branch = "release",
    config = function()
        vim.opt.backup = false
        vim.opt.writebackup = false
        vim.opt.updatetime = 300
        vim.opt.signcolumn = "yes"
        local keyset = vim.keymap.set
        function _G.check_back_space()
            local col = vim.fn.col(".") - 1
            return col == 0 or vim.fn.getline("."):sub(col, col):match("%s") ~= nil
        end

        local opts = { silent = true, noremap = true, expr = true, replace_keycodes = false }
        keyset(
            "i",
            "<TAB>",
            'coc#pum#visible() ? coc#pum#next(1) : v:lua.check_back_space() ? "<TAB>" : coc#refresh()',
            opts
        )

        keyset(
            "i",
            "<RETURN>",
            [[coc#pum#visible() ? coc#pum#confirm() : "\<C-g>u\<CR>\<c-r>=coc#on_enter()\<CR>"]],
            opts
        )

        function _G.show_docs()
            local cw = vim.fn.expand("<cword>")
            if vim.fn.index({ "vim", "help" }, vim.bo.filetype) >= 0 then
                vim.api.nvim_command("h " .. cw)
            elseif vim.api.nvim_eval("coc#rpc#ready()") then
                vim.fn.CocActionAsync("doHover")
            else
                vim.api.nvim_command("!" .. vim.o.keywordprg .. " " .. cw)
            end
        end

        keyset("n", "<leader>k", "<CMD>lua _G.show_docs()<CR>", { silent = true })
        keyset("v", "<leader>f", "<Plug>(coc-format-selected)", { silent = true })
        keyset("n", "<leader>gd", "<Plug>(coc-definition)", { silent = true })
        keyset("n", "<leader>gy", "<Plug>(coc-type-definition)", { silent = true })
        keyset("n", "<leader>gi", "<Plug>(coc-implementation)", { silent = true })
        keyset("n", "<leader>gr", "<Plug>(coc-references)", { silent = true })
        keyset("n", "<leader>a", "<Plug>(coc-codeaction-selected)", opts)
        keyset("n", "<leader>qf", "<Plug>(coc-fix-current)", opts)
    end,
}
