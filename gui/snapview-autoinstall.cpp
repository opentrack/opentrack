#include "snapview-table.hpp"

#include "logic/snapview.hpp"
#include "spline/spline-widget.hpp"

#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QTabWidget>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace
{

struct curve_binding final
{
    const char* object_name;
    Axis axis;
    bool alt;
};

constexpr curve_binding mapping_widgets[] = {
    { "rxconfig",     Yaw,   false },
    { "ryconfig",     Pitch, false },
    { "rzconfig",     Roll,  false },
    { "txconfig",     TX,    false },
    { "tyconfig",     TY,    false },
    { "tzconfig",     TZ,    false },

    { "rxconfig_alt", Yaw,   true  },
    { "ryconfig_alt", Pitch, true  },
    { "rzconfig_alt", Roll,  true  },
    { "txconfig_alt", TX,    true  },
    { "tyconfig_alt", TY,    true  },
    { "tzconfig_alt", TZ,    true  },
};

QString snapview_tab_text()
{
    return QCoreApplication::translate("Snap View_auto_installer", "Snap View");
}

snapview_table* find_snapview_table(QTabWidget* tabs)
{
    if (!tabs)
        return nullptr;

    for (int i = 0; i < tabs->count(); i++)
    {
        if (auto* table = qobject_cast<snapview_table*>(tabs->widget(i)))
            return table;
    }

    return tabs->findChild<snapview_table*>(QStringLiteral("snapview_table"));
}

void remove_duplicate_snapview_tabs(QTabWidget* tabs, snapview_table* keep)
{
    if (!tabs)
        return;

    for (int i = tabs->count() - 1; i >= 0; i--)
    {
        QWidget* widget = tabs->widget(i);

        if (widget == keep)
            continue;

        if (qobject_cast<snapview_table*>(widget))
        {
            tabs->removeTab(i);
            widget->deleteLater();
        }
    }
}

int target_snapview_index(QTabWidget* tabs)
{
    if (!tabs)
        return 0;

    // In options-dialog.ui the built-in tabs start as:
    //   0: Shortcuts
    //   1: Output
    // Insert snapview after Shortcuts and before Output without depending on localized tab text.
    return std::min(1, tabs->count());
}

void register_widget_points(spline_widget* widget)
{
    if (!widget)
        return;

    const Axis axis = widget->snapview_axis();
    if (axis == NonAxis)
        return;

    snapview::instance().register_curve(axis,
                                       widget->snapview_alt(),
                                       widget->snapview_points());
    widget->update();
}

void configure_mapping_dialog(QWidget* dialog)
{
    if (!dialog || dialog->property("Snap View.mapping.installed").toBool())
        return;

    dialog->setProperty("Snap View.mapping.installed", true);

    for (const curve_binding& binding : mapping_widgets)
    {
        spline_widget* widget = dialog->findChild<spline_widget*>(QString::fromLatin1(binding.object_name));
        if (!widget)
            continue;

        widget->set_snapview_axis(binding.axis, binding.alt);

        QObject::connect(widget, &spline_widget::snapview_points_changed,
                         widget,
                         [widget]
                         {
                             register_widget_points(widget);
                         });

        QTimer::singleShot(0, widget, [widget] { register_widget_points(widget); });
    }
}

void configure_options_dialog(QWidget* dialog)
{
    if (!dialog)
        return;

    QTabWidget* tabs = dialog->findChild<QTabWidget*>();
    if (!tabs)
        return;

    snapview_table* table = find_snapview_table(tabs);
    if (!table)
        table = new snapview_table(tabs);

    table->setObjectName(QStringLiteral("snapview_table"));

    remove_duplicate_snapview_tabs(tabs, table);

    const int current_index = tabs->indexOf(table);
    if (current_index >= 0)
        tabs->removeTab(current_index);

    const int insert_index = target_snapview_index(tabs);
    tabs->insertTab(insert_index, table, snapview_tab_text());
    tabs->setTabText(tabs->indexOf(table), snapview_tab_text());

    QTimer::singleShot(0, table, &snapview_table::reload);
}

class snapview_auto_installer final : public QObject
{
public:
    bool eventFilter(QObject* object, QEvent* event) override
    {
        if (!object || !event)
            return QObject::eventFilter(object, event);

        const QEvent::Type type = event->type();
        if (type != QEvent::Show && type != QEvent::ChildAdded && type != QEvent::LanguageChange)
            return QObject::eventFilter(object, event);

        auto* widget = qobject_cast<QWidget*>(object);
        if (!widget)
            return QObject::eventFilter(object, event);

        const QByteArray class_name = widget->metaObject()->className();

        if (class_name == QByteArrayLiteral("mapping_dialog"))
            QTimer::singleShot(0, widget, [widget] { configure_mapping_dialog(widget); });
        else if (class_name == QByteArrayLiteral("options_dialog"))
            QTimer::singleShot(0, widget, [widget] { configure_options_dialog(widget); });

        return QObject::eventFilter(object, event);
    }
};

void install_snapview_auto_installer()
{
    static snapview_auto_installer installer;

    if (QCoreApplication* app = QCoreApplication::instance())
        app->installEventFilter(&installer);
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(install_snapview_auto_installer)
