/*
 * Copyright 2012 Canonical Ltd.
 *
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "indicator-printers.h"
#include "indicator-menu-item.h"
#include "dbus-names.h"

#include <gtk/gtk.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-image-helper.h>

#include <libdbusmenu-gtk3/menu.h>
#include <libdbusmenu-gtk3/menuitem.h>


INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_PRINTERS_TYPE)


G_DEFINE_TYPE (IndicatorPrinters, indicator_printers, INDICATOR_OBJECT_TYPE)

#define INDICATOR_PRINTERS_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_PRINTERS_TYPE, IndicatorPrintersPrivate))


struct _IndicatorPrintersPrivate
{
    IndicatorObjectEntry entry;
};


static void
dispose (GObject *object)
{
    IndicatorPrintersPrivate *priv = INDICATOR_PRINTERS_GET_PRIVATE (object);
    g_clear_object (&priv->entry.menu);
    g_clear_object (&priv->entry.image);
    G_OBJECT_CLASS (indicator_printers_parent_class)->dispose (object);
}


static GList *
get_entries (IndicatorObject *io)
{
    IndicatorPrintersPrivate *priv = INDICATOR_PRINTERS_GET_PRIVATE (io);
    return g_list_append (NULL, &priv->entry);
}


static void
indicator_printers_class_init (IndicatorPrintersClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    IndicatorObjectClass *io_class = INDICATOR_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (IndicatorPrintersPrivate));

    object_class->dispose = dispose;

    io_class->get_entries = get_entries;
}


static gboolean
is_string_property (const gchar *name,
                    const gchar *prop,
                    GVariant *value)
{
    return !g_strcmp0 (name, prop) &&
           g_variant_is_of_type (value, G_VARIANT_TYPE_STRING);
}


static void
indicator_prop_change_cb (DbusmenuMenuitem *mi,
                          gchar *prop,
                          GVariant *value,
                          gpointer user_data)
{
    IndicatorMenuItem *menuitem = user_data;

    if (is_string_property (prop, "indicator-label", value))
        indicator_menu_item_set_label (menuitem, g_variant_get_string (value, NULL));
    else if (is_string_property (prop, "indicator-right", value))
        indicator_menu_item_set_right (menuitem, g_variant_get_string (value, NULL));
}


static gboolean
new_indicator_item (DbusmenuMenuitem *newitem,
                    DbusmenuMenuitem *parent,
                    DbusmenuClient *client,
                    gpointer user_data)
{
    GtkWidget *menuitem;
    const gchar *text, *right_text;
    gboolean is_lozenge;

    text = dbusmenu_menuitem_property_get (newitem, "indicator-label");
    right_text = dbusmenu_menuitem_property_get (newitem, "indicator-right");
    is_lozenge = dbusmenu_menuitem_property_get_bool (newitem, "indicator-right-is-lozenge");

    menuitem = g_object_new (INDICATOR_TYPE_MENU_ITEM,
                             "label", text,
                             "right", right_text,
                             "right-is-lozenge", is_lozenge,
                             NULL);
    gtk_widget_show_all (menuitem);

    dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client),
                                    newitem,
                                    GTK_MENU_ITEM (menuitem),
                                    parent);

    g_signal_connect(G_OBJECT(newitem),
                     "property-changed",
                     G_CALLBACK(indicator_prop_change_cb),
                     menuitem);

    return TRUE;
}


static void
indicator_printers_init (IndicatorPrinters *io)
{
    IndicatorPrintersPrivate *priv = INDICATOR_PRINTERS_GET_PRIVATE (io);
    DbusmenuGtkMenu *menu;
    DbusmenuClient *client;
    GtkImage *image;

    menu = dbusmenu_gtkmenu_new(INDICATOR_PRINTERS_DBUS_NAME,
                                INDICATOR_PRINTERS_DBUS_OBJECT_PATH);

    client = DBUSMENU_CLIENT (dbusmenu_gtkmenu_get_client (menu));
    dbusmenu_client_add_type_handler(client,
                                     "indicator-item",
                                     new_indicator_item);

    image = indicator_image_helper ("printer-symbolic");
    gtk_widget_show (GTK_WIDGET (image));

    priv->entry.name_hint = PACKAGE_NAME;
    priv->entry.accessible_desc = "Printers";
    priv->entry.menu = GTK_MENU (g_object_ref_sink (menu));
    priv->entry.image = g_object_ref_sink (image);
}


IndicatorPrinters *
indicator_printers_new (void)
{
    return g_object_new (INDICATOR_PRINTERS_TYPE, NULL);
}

