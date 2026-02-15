-- .nvim.lua
vim.g.rustaceanvim = {
  server = {
    settings = {
      ["rust-analyzer"] = {
        -- Force-clear the incorrect paths
        linkedProjects = {},
        -- Enable automatic discovery of the root Cargo.toml
        cargo = {
          allFeatures = true,
        },
      },
    },
  },
}
