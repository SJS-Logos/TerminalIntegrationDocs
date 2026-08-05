# Creating Documentation

Guidelines for adding solution documentation to a repository so it is automatically
published to Confluence.

## Structure

Documentation lives in a `docs` folder at the repository root:

```
ProjectName
├── docs
│   ├── diagrams
│   ├── source files
│   └── documentation.md
```

- Markdown files in the root of `docs` are published as pages.
- Each subfolder under `docs` that contains markdown files becomes a sub-page
  named after the folder, with its markdown files as child pages.

## Requirements

- `.md` files must be placed in the root of the `docs` folder (or in a
  subfolder to create a sub-page).
- The first header (`#`) of each `.md` file is used as the page title.
- Development-specific information belongs in the project's root `readme.md`
  and is **not** published to Confluence.
- Solution documentation that should reach Confluence belongs in `docs/`.
- If needed, create a matching folder in Confluence to host the documentation.

A template for the `readme.md` can be found in the
[Templates README.md](https://logospaymentsolutions.atlassian.net/wiki/spaces/AAIOB/pages/1117356039/Setting+up+GitHub+to+Confluence+documentation).

## Workflow

Add the following job to the workflow that runs on merge to `development` so
documentation stays in sync with Confluence:

```yaml
sync-readme-to-confluence:
  runs-on: ubuntu-latest
  name: Sync to Confluence
  steps:
    - name: Checkout code
      uses: actions/checkout@v2

    - name: Sync to Confluence
      uses: markdown-confluence/publish-action@v5
      with:
        atlassianusername: ${{ vars.atlassian_username }}
        atlassianapitoken: ${{ secrets.atlassian_api_token }}
```

## Permissions

Publishing is performed as the Atlassian user referenced by
`vars.atlassian_username`, using the API token in
`secrets.atlassian_api_token`. That user must have **Add page** rights in the
target Confluence space; without this permission the workflow will fail to
create or update pages. 

> The user currently configured for `vars.atlassian_username` is 
> `logosdevsa@logos.dk`.

## Configuration

Add a `.markdown-confluence.json` file to the root of the repository:

```json
{
  "confluenceBaseUrl": "https://logospaymentsolutions.atlassian.net",
  "confluenceParentId": "YOURPARENTID",
  "folderToPublish": "docs",
  "firstHeadingPageTitle": true
}
```

## Finding the Parent ID

Right-click the target folder in Confluence and copy the link. The space key and
parent id are the last two segments of the URL.

Example:

```
https://logospaymentsolutions.atlassian.net/wiki/spaces/AAIOB/folder/1104936975
```

- `YOURPARENTID` = `1104936975`
