# Contributing Guidelines

Thank you for your interest in contributing to the Smart Debt Recovery System!  
This document describes the workflow, coding standards, and pull request process used in this project.

---

## 1. Workflow

This project follows an enterprise-grade Git workflow:

- No one is allowed to push directly to the `main` branch.
- All changes must go through a Pull Request (PR).
- Every PR must be reviewed and approved before merging.
- One task = one branch = one PR.
- Keep your branches focused on a single responsibility.

---

## 2. Creating a Branch

Branch names follow the structure:

```bash
<type>/<short-description>
```
### Allowed types:
- `feat/` — new features  
- `fix/` — bug fixes  
- `refactor/` — code improvements without changing behavior  
- `docs/` — documentation updates  
- `chore/` — scripts, config, CI/CD, build tasks  

### Examples
```bash
feat/borrower-crud-api
fix/risk-score-threshold
refactor/account-service
chore/update-cmake-config
```

To create a branch:
```bash
git checkout -b feat/borrower-crud-api
```
---

## 3. Commit Guidelines
This project uses **Conventional Commits:**
```bash
<type>(optional-scope): short description
```

### Valid commit types:
- `feat:` add features
- `fix:` fix bugs
- `refactor:` restructure code
- `docs:` update documentation
- `test:` add or modify tests
- `chore:` maintenance tasks

### Examples
```bash
feat(borrower): add CRUD endpoints
fix(risk): correct score threshold calculation
chore(project): add project skeleton script
```
---

## 4. Pull Request Guidelines
When finishing your work, create a PR targeting the main branch.

### A Pull Request must include:
- A clear and meaningful title
- Description of what was changed
- Screenshots/logs for UI or major changes (if applicable)

### A PR will NOT be merged if:
- It does not pass CI checks (if enabled)
- It lacks approval from the project lead
- It contains multiple unrelated changes
- It introduces unnecessary complexity or duplicated code

---

## 5. Code Style
This project follows modern C++ practices (C++20+):
- Prefer `std::unique_ptr` / `std::shared_ptr` appropriately
- Avoid raw pointers unless strictly required
- Templates should remain header-only
- One class per file
- Functions should ideally stay under ~60 lines
- Naming conventions:
    - Classes: `PascalCase`
    - Functions/variables: `camelCase`
    - Constants: `ALL_CAPS`

Keep code readable, maintainable, and consistent.

---

## 6. Testing
- Each module should provide its own tests under `tests/`
- All tests must pass before submitting a PR
- New features should include test coverage when possible
- Tests must be reproducible and deterministic.

---

## 7. Directory Structure
Follow the established project layout:
```txt
service/
 ├── include/
 ├── src/
 ├── tests/
 └── CMakeLists.txt
```
Do not introduce new top-level directories without discussion in an issue or PR.

New services should follow the same internal structure for consistency.

---

## 8. Review Guidelines
When reviewing a PR:
- Verify correctness and logic
- Check for performance issues
- Ensure there is no duplicated or dead code
- Confirm code readability and maintainability
- Suggest improvements when appropriate
- Check commit messages follow the Conventional Commits specification.
---

Thank you for contributing to this project!

If you have questions, feel free to open an Issue or contact the project lead.