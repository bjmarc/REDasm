#include "graphview.h"
#include <QDragEnterEvent>
#include <QFontDatabase>
#include <QApplication>
#include <QJsonDocument>
#include <QAction>
#include "../../themeprovider.h"
#include "../../redasmsettings.h"

GraphView::GraphView(QWidget *parent): QWebEngineView(parent)
{
    connect(this, &GraphView::loadFinished, this, &GraphView::initializePage);

    m_contextmenu = new QMenu(this);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->load(QUrl("qrc:/web/graph.html"));

    connect(this, &GraphView::customContextMenuRequested, this, [&](const QPoint&) {
        m_contextmenu->exec(QCursor::pos());
    });
}

void GraphView::setGraph(const REDasm::Graphing::Graph &graph)
{
    this->page()->runJavaScript("GraphView.initGraph();");
    this->generateNodes(graph);
    this->generateEdges(graph);
    this->page()->runJavaScript("GraphView.renderGraph();");
}

void GraphView::focusOnLine(int line) { this->page()->runJavaScript("GraphView.focusOnLine(" + QString::number(line) + ");"); }
void GraphView::dragEnterEvent(QDragEnterEvent *e) { e->ignore(); }
QString GraphView::getNodeTitle(const REDasm::Graphing::Node *n) const { Q_UNUSED(n) return QString(); }
QColor GraphView::getEdgeColor(const REDasm::Graphing::Node *from, const REDasm::Graphing::Node *to) const { Q_UNUSED(from) Q_UNUSED(to) return QColor(Qt::black); }
void GraphView::zoomOn(int line) { /* this->page()->runJavaScript(QString("GraphView.zoomOn(%1);").arg(line)); */ }
void GraphView::appendCSS(const QString &css) { this->page()->runJavaScript(QString("GraphView.appendCss('%1');").arg(css)); }

void GraphView::initializePage()
{
    REDasmSettings settings;
    QFont font = settings.currentFont();
    QPalette palette = qApp->palette();

    m_contextmenu->clear();
    this->configureActions();
    this->page()->runJavaScript("GraphView.initPage();");

    QString generalcss = "html {"
                             "cursor: default;"
                             "font-family:" + font.family() + ";" +
                             "font-size:" + QString::number(settings.currentFontSize()) + "pt;" +
                             "color:" + palette.color(QPalette::WindowText).name() + ";" //+
                             "background-color:" + THEME_VALUE_COLOR("graph_bg") + ";" +
                         "}"
                         "html, body {"
                            "overflow: hidden;"
                            "margin: 0px;"
                         "}";

    QString blockcss =  ".nodetitle { "
                            "text-align: center;"
                            "margin-bottom: 4px;"
                            "border: 1px solid " + palette.color(QPalette::WindowText).name() + ";"
                            "background-color: " + palette.color(QPalette::Window).name() + ";"
                            "color: " + palette.color(QPalette::WindowText).name() + ";"
                        "}"
                         ".node rect {"
                            "fill: " + palette.color(QPalette::Base).name() + ";" +
                            "stroke: " + palette.color(QPalette::WindowText).name() + ";" +
                            "stroke-width: 3;"
                            "filter: url(#dropshadow);"
                        "}"
                        ".edgepath path {"
                            "stroke-width: 1.5;"
                        "}"
                        ".edgepath path:hover {"
                            "stroke-width: 2.5;"
                            //"stroke: " + palette.color(QPalette::Highlight).name() + " !important;"
                        "}";

    this->appendCSS(generalcss);
    this->appendCSS(blockcss);
}

void GraphView::generateNodes(const REDasm::Graphing::Graph &graph)
{
    for(auto& n : graph)
    {
        QString title = this->getNodeTitle(n.get()).toHtmlEscaped(),
                content = this->getNodeContent(n.get()).toHtmlEscaped();

        this->page()->runJavaScript(QString("GraphView.setNode(%1, \"%2\", \"%3\");")
            .arg(n->id)
            .arg(title, content)
        );
    }
}

void GraphView::generateEdges(const REDasm::Graphing::Graph &graph)
{
    for(auto& n : graph)
    {
        const REDasm::Graphing::Graph::AdjacencyList& edges = graph.edges(n);

        for(auto& e : edges)
        {
            QColor color = this->getEdgeColor(n.get(), e);
            this->page()->runJavaScript(QString("GraphView.setEdge(%1, %2, '%3');")
                                        .arg(n->id)
                                        .arg(e->id)
                                        .arg(color.name()));
        }
    }
}

void GraphView::configureActions()
{
    m_contextmenu->addAction(this->pageAction(QWebEnginePage::Copy));
}
