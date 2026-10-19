#!/usr/bin/env bash
# Build kubisat_operator (Release) and install it for the current user
# so it appears in the desktop applications menu.
set -euo pipefail

APP_ID="kubisat-operator"
APP_NAME="KubiSat Operator"
BINARY_NAME="kubisat_operator"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
PUBLISH_DIR="${REPO_ROOT}/publish/linux"

INSTALL_ROOT="${HOME}/.local/share/${APP_ID}"
BIN_LINK="${HOME}/.local/bin/${APP_ID}"
DESKTOP_FILE="${HOME}/.local/share/applications/${APP_ID}.desktop"
ICON_DIR="${HOME}/.local/share/icons/hicolor/scalable/apps"
ICON_FILE="${ICON_DIR}/${APP_ID}.svg"
SOURCE_ICON="${REPO_ROOT}/assets/kubisat-operator.svg"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--uninstall] [--framework-dependent]

  (default)  Publish Release build and install for the current user.
  --uninstall
             Remove the installed app, launcher, and desktop entry.
  --framework-dependent
             Publish against the installed .NET runtime instead of
             bundling a self-contained build (smaller install).
EOF
}

detect_rid() {
  local arch
  arch="$(uname -m)"
  case "${arch}" in
    x86_64|amd64) echo "linux-x64" ;;
    aarch64|arm64) echo "linux-arm64" ;;
    armv7l) echo "linux-arm" ;;
    *)
      echo "Unsupported architecture: ${arch}" >&2
      exit 1
      ;;
  esac
}

uninstall_app() {
  echo "Removing ${APP_NAME}..."
  rm -rf "${INSTALL_ROOT}"
  rm -f "${BIN_LINK}"
  rm -f "${DESKTOP_FILE}"
  rm -f "${ICON_FILE}"

  if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "${HOME}/.local/share/applications" >/dev/null 2>&1 || true
  fi
  if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor" >/dev/null 2>&1 || true
  fi

  echo "Uninstalled."
}

SELF_CONTAINED=true
ACTION=install

for arg in "$@"; do
  case "${arg}" in
    -h|--help)
      usage
      exit 0
      ;;
    --uninstall)
      ACTION=uninstall
      ;;
    --framework-dependent)
      SELF_CONTAINED=false
      ;;
    *)
      echo "Unknown option: ${arg}" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ "${ACTION}" == "uninstall" ]]; then
  uninstall_app
  exit 0
fi

if ! command -v dotnet >/dev/null 2>&1; then
  echo "dotnet SDK is required but was not found in PATH." >&2
  exit 1
fi

RID="$(detect_rid)"
echo "Building ${APP_NAME} (Release, ${RID}, self-contained=${SELF_CONTAINED})..."

rm -rf "${PUBLISH_DIR}"
mkdir -p "${PUBLISH_DIR}"

PUBLISH_ARGS=(
  publish
  "${REPO_ROOT}/kubisat_operator.csproj"
  -c Release
  -r "${RID}"
  -o "${PUBLISH_DIR}"
  --self-contained "${SELF_CONTAINED}"
  -p:PublishReadyToRun=true
  -p:DebugType=None
  -p:DebugSymbols=false
)

dotnet "${PUBLISH_ARGS[@]}"

if [[ ! -x "${PUBLISH_DIR}/${BINARY_NAME}" ]]; then
  echo "Publish succeeded but executable was not found: ${PUBLISH_DIR}/${BINARY_NAME}" >&2
  exit 1
fi

echo "Installing to ${INSTALL_ROOT}..."
mkdir -p "${INSTALL_ROOT}"
mkdir -p "$(dirname "${BIN_LINK}")"
mkdir -p "$(dirname "${DESKTOP_FILE}")"
mkdir -p "${ICON_DIR}"

rm -rf "${INSTALL_ROOT}"
mkdir -p "${INSTALL_ROOT}"
cp -a "${PUBLISH_DIR}/." "${INSTALL_ROOT}/"
chmod +x "${INSTALL_ROOT}/${BINARY_NAME}"

ln -sfn "${INSTALL_ROOT}/${BINARY_NAME}" "${BIN_LINK}"

if [[ -f "${SOURCE_ICON}" ]]; then
  cp "${SOURCE_ICON}" "${ICON_FILE}"
fi

cat > "${DESKTOP_FILE}" <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=${APP_NAME}
Comment=Operator stacji naziemnej KubiSat
Exec=${BIN_LINK}
Icon=${APP_ID}
Terminal=false
Categories=Utility;HamRadio;Science;
StartupNotify=true
EOF
chmod 644 "${DESKTOP_FILE}"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${HOME}/.local/share/applications" >/dev/null 2>&1 || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor" >/dev/null 2>&1 || true
fi

echo
echo "Installed successfully."
echo "  App dir : ${INSTALL_ROOT}"
echo "  Launcher: ${BIN_LINK}"
echo "  Menu    : ${DESKTOP_FILE}"
echo
echo "Start from the apps menu as \"${APP_NAME}\", or run: ${APP_ID}"
if ! groups | tr ' ' '\n' | grep -qx dialout; then
  echo
  echo "Note: to access serial ports, add your user to the dialout group:"
  echo "  sudo usermod -aG dialout \"\$USER\""
  echo "then log out and back in."
fi
