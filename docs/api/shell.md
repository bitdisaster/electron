# shell

> Manage files and URLs using their default applications.

Process: [Main](../glossary.md#main-process), [Renderer](../glossary.md#renderer-process) (non-sandboxed only)

The `shell` module provides functions related to desktop integration.

An example of opening a URL in the user's default browser:

```javascript
const { shell } = require('electron')

shell.openExternal('https://github.com')
```

**Note:** While the `shell` module can be used in the renderer process, it will not function in a sandboxed renderer.

## Methods

The `shell` module has the following methods:

### `shell.showItemInFolder(fullPath)`

* `fullPath` String

Show the given file in a file manager. If possible, select the file.

### `shell.openPath(path)`

* `path` String

Returns `Promise<String>` - Resolves with an string containing the error message corresponding to the failure if a failure occurred, otherwise "".

Open the given file in the desktop's default manner.

### `shell.openExternal(url[, options])`

* `url` String - Max 2081 characters on windows.
* `options` Object (optional)
  * `activate` Boolean (optional) _macOS_ - `true` to bring the opened application to the foreground. The default is `true`.
  * `workingDirectory` String (optional) _Windows_ - The working directory.

Returns `Promise<void>`

Open the given external protocol URL in the desktop's default manner. (For example, mailto: URLs in the user's default mail agent).

### `shell.moveItemToTrash(fullPath[, deleteOnFail])`

* `fullPath` String
* `deleteOnFail` Boolean (optional) - Whether or not to unilaterally remove the item if the Trash is disabled or unsupported on the volume. _macOS_

Returns `Boolean` - Whether the item was successfully moved to the trash or otherwise deleted.

Move the given file to trash and returns a boolean status for the operation.

### `shell.beep()`

Play the beep sound.

### `shell.writeShortcutLink(shortcutPath[, operation], options)` _Windows_

* `shortcutPath` String
* `operation` String (optional) - Default is `create`, can be one of following:
  * `create` - Creates a new shortcut, overwriting if necessary.
  * `update` - Updates specified properties only on an existing shortcut.
  * `replace` - Overwrites an existing shortcut, fails if the shortcut doesn't
    exist.
* `options` [ShortcutDetails](structures/shortcut-details.md)

Returns `Boolean` - Whether the shortcut was created successfully.

Creates or updates a shortcut link at `shortcutPath`.

### `shell.readShortcutLink(shortcutPath)` _Windows_

* `shortcutPath` String

Returns [`ShortcutDetails`](structures/shortcut-details.md)

Resolves the shortcut link at `shortcutPath`.

An exception will be thrown when any error happens.

### `shell.registerInActionCenter(toastActivatorClsid)` _Windows_

* `toastActivatorClsid` String - A GUID representing the Toast Activator CLSID.

Registers the App to participate in the Action Center with a `toastActivatorClsid`.
The registration is necessary to enable activation via the Action Center such as
click, reply, button clicks or multiple choices. The provided Toast Activator CLSID
must also be used in tandem with App User Model Id written to a shortcut in the start menu.
See [`ShortcutDetails`](structures/shortcut-details.md). The shortcut and Action Center
registration are ideally created at install time and removed on uninstall. THis can also
be achieved but common installer frameworks such as APPX or MSI.
See details [`Win32 Notifications`]https://docs.microsoft.com/en-us/windows/uwp/design/shell/tiles-and-notifications/send-local-toast-desktop-cpp-wrl

### `shell.unregisterInActionCenter()` _Windows_

Removes the registration in Action Center. Should be called once on uninstall.
