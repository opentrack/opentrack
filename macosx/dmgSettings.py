# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import biplist
import os.path

macosxPath = defines.get('macosxPath')
installPath = defines.get('installPath')
application = installPath + 'opentrack.app'
appname = os.path.basename(application)

def icon_from_app(app_path):
    plist_path = os.path.join(app_path, 'Contents', 'Info.plist')
    plist = biplist.readPlist(plist_path)
    icon_name = plist['CFBundleIconFile']
    icon_root,icon_ext = os.path.splitext(icon_name)
    if not icon_ext:
        icon_ext = '.icns'
    icon_name = icon_root + icon_ext
    return os.path.join(app_path, 'Contents', 'Resources', icon_name)

# .. Basics ....................................................................

# Uncomment to override the output filename
# filename = 'test.dmg'

# Uncomment to override the output volume name
volume_name = 'Opentrack'

# Volume format (see hdiutil create -help)
format = defines.get('format', 'UDBZ')

# Volume size
size = defines.get('size', None)

# Files to include
files = [ application,
         installPath + 'Documentation',
         installPath + 'X-Plane-Plugin',
         installPath + 'source-code',
         installPath + 'thirdparty'
         ]

# Symlinks to create
symlinks = { 'Applications': '/Applications' }

# Volume icon
#
# You can either define icon, in which case that icon file will be copied to the
# image, *or* you can define badge_icon, in which case the icon file you specify
# will be used to badge the system's Removable Disk icon
#
#icon = '/path/to/icon.icns'
badge_icon = icon_from_app(application)

# Where to put the icons
icon_locations = {
    'opentrack.app': (200, 180),
    'Applications': (420, 180),
    'Documentation': (20,40),
    'source-code': (220,40),
    'X-Plane-Plugin': (420,40),
    'thirdparty': (620,40)
    }

background =  macosxPath + 'dmgbackground.png'

show_status_bar = False
show_tab_view = False
show_toolbar = False
show_pathbar = False
show_sidebar = False
sidebar_width = 180

window_rect = ((200, 120), (800, 450))

default_view = 'icon-view'

show_icon_preview = False

include_icon_view_settings = 'auto'
include_list_view_settings = 'auto'

# .. Icon view configuration ...................................................
arrange_by = None
grid_offset = (0, 0)
grid_spacing = 100
scroll_position = (0, 0)
label_pos = 'bottom' # or 'right'
text_size = 16
icon_size = 80

# .. List view configuration ...................................................
list_icon_size = 16
list_text_size = 12
list_scroll_position = (0, 0)
list_sort_by = 'name'
list_use_relative_dates = True
list_calculate_all_sizes = False,
list_columns = ('name', 'date-modified', 'size', 'kind', 'date-added')
list_column_widths = {
    'name': 300,
    'date-modified': 181,
    'date-created': 181,
    'date-added': 181,
    'date-last-opened': 181,
    'size': 97,
    'kind': 115,
    'label': 100,
    'version': 75,
    'comments': 300,
    }
list_column_sort_directions = {
    'name': 'ascending',
    'date-modified': 'descending',
    'date-created': 'descending',
    'date-added': 'descending',
    'date-last-opened': 'descending',
    'size': 'descending',
    'kind': 'ascending',
    'label': 'ascending',
    'version': 'ascending',
    'comments': 'ascending',
    }
