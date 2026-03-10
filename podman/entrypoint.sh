#!/bin/bash
# Entrypoint for the Hermes container.
# Installs Claude Code on first run if not already present, then execs
# the supplied command (defaults to bash).

export PATH="$HOME/.local/bin:$PATH"

if ! command -v claude &>/dev/null; then
  echo "Installing Claude Code..."
  curl -fsSL https://claude.ai/install.sh | bash
fi

exec "$@"
