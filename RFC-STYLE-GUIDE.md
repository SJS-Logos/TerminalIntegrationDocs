# RFC Style Guide

This document defines the formatting conventions for all RFC documents in this repository.
Use this as an instruction set when creating or editing RFCs.

---

## 1. Front Matter

Every RFC MUST start with a level-1 heading followed by bold metadata fields with trailing double-spaces for line breaks:

```markdown
# RFC: <Title>

**Version:** <major>.<minor>  
**Status:** Draft  
```

- Title MUST be prefixed with `RFC:`.
- Version MUST NOT include parenthetical suffixes like `(Draft 1)`.
- Status MUST be one of: `Draft`, `Review`, `Accepted`, `Deprecated`.

---

## 2. Section Separators

Use markdown horizontal rules (`---`) between all top-level sections.

Do NOT use unicode characters like `⸻` or `———`.

```markdown
---
```

---

## 3. Headings

### 3.1 Top-Level Sections

Use `#` (h1) with a numbered prefix:

```markdown
# 1. Introduction
# 2. Abstract
# 3. Normative Language
```

### 3.2 Sub-Sections

Use `##` (h2) with dotted numbering:

```markdown
## 5.1 Components
## 5.2 Infrastructure Dependencies
```

### 3.3 Sub-Sub-Sections

Use `###` (h3) for named blocks within a sub-section:

```markdown
### Requirements
### Examples
### Response
```

Sub-sub-sections do NOT use numbered prefixes.

---

## 4. Normative Language Section

The normative language section MUST appear early (typically section 2 or 3) and MUST use bold for all RFC 2119 keywords:

```markdown
# 3. Normative Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, and **MAY** are to be interpreted as described in RFC 2119.
```

Throughout the document, these keywords are written in UPPERCASE but are NOT bolded in running text — only in the normative language declaration itself.

---

## 5. Lists

### 5.1 Bullet Lists

Use `-` as the list marker. Each item MUST end with two trailing spaces for consistent line breaks:

```markdown
- First item  
- Second item  
- Third item  
```

### 5.2 Numbered Lists

Use `1.`, `2.`, etc. Each item MUST end with two trailing spaces:

```markdown
1. First step  
2. Second step  
3. Third step  
```

### 5.3 Nested Lists

Indent nested items by 2 spaces:

```markdown
- Parent item  
  - Child item  
  - Child item  
```

---

## 6. Code Blocks

### 6.1 Inline Code

Use backticks for identifiers, values, paths, headers, and commands:

```markdown
The `DeviceId` is stored in `device:{id}:queue`.
```

### 6.2 Fenced Code Blocks

Use triple backticks with a language identifier:

````markdown
```json
{
  "id": "example"
}
```
````

### 6.3 Indented Code Blocks

Use 4-space indentation for short inline definitions like key patterns or endpoints:

```markdown
    GET /api/poll/{deviceAppId}
    SET device:{deviceAppId}:podIp "{podIp}" EX 300
```

---

## 7. Emphasis and Formatting

- **Bold** (`**text**`) for:
  - Component names in lists (e.g., `**LongPolling.API**`)
  - Key terms being defined
  - Metadata field labels in front matter
- *Italic* is generally NOT used.
- UPPERCASE for RFC 2119 keywords in running text (MUST, SHALL, etc.) — without bold.

---

## 8. Paragraphs and Spacing

- One blank line between paragraphs.
- One blank line before and after code blocks, lists, and separators.
- No trailing whitespace on blank lines.
- Trailing double-space (`  `) on list items and multi-line metadata for explicit line breaks.

---

## 9. Section Ordering Convention

The following order is RECOMMENDED. Not all sections apply to every RFC:

1. Introduction / Abstract
2. Normative Language
3. Motivation (if applicable)
4. Scope (In Scope / Out of Scope)
5. Core technical sections (varies per RFC)
6. Design Rationale (if applicable)
7. Failure Scenarios (if applicable)
8. Accepted Risks (if applicable)
9. Security Considerations
10. Configuration (if applicable)
11. Versioning (if applicable)
12. Compliance (if applicable)
13. Open Questions (if applicable)
14. References (if applicable)

---

## 10. Definitions and Blockquotes

Use blockquotes (`>`) for formal definitions:

```markdown
An application is:

> An OS-level process or execution context performing a specific computational task.
```

---

## 11. Checklist

Before submitting an RFC, verify:

- [ ] Title starts with `# RFC:`
- [ ] Front matter uses `**bold**` labels
- [ ] All section separators are `---`
- [ ] Top-level sections use `#` with numbered prefix
- [ ] Sub-sections use `##` with dotted numbering
- [ ] Normative language section declares bold keywords
- [ ] List items end with trailing double-spaces
- [ ] Code blocks have language identifiers
- [ ] No unicode decorative characters
