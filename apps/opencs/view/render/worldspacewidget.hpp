#ifndef OPENCS_VIEW_WORLDSPACEWIDGET_H
#define OPENCS_VIEW_WORLDSPACEWIDGET_H

#include "scenewidget.hpp"

#include "navigation1st.hpp"
#include "navigationfree.hpp"
#include "navigationorbit.hpp"
#include <apps/opencs/model/doc/document.hpp>

namespace CSMWorld
{
    class UniversalId;
}
namespace CSVWorld
{
    class SceneToolMode;
    class SceneToolbar;
}

namespace CSVRender
{
    class WorldspaceWidget : public SceneWidget
    {
            Q_OBJECT

            CSVRender::Navigation1st m1st;
            CSVRender::NavigationFree mFree;
            CSVRender::NavigationOrbit mOrbit;

        public:

        enum dropType
        {
            cellsMixed,
            cellsInterior,
            cellsExterior,
            notCells
        };

            WorldspaceWidget (const CSMDoc::Document& document, QWidget *parent = 0);

            CSVWorld::SceneToolMode *makeNavigationSelector (CSVWorld::SceneToolbar *parent);
            ///< \attention The created tool is not added to the toolbar (via addTool). Doing that
            /// is the responsibility of the calling function.

            void selectDefaultNavigationMode();

            static dropType getDropType(const std::vector<CSMWorld::UniversalId>& data);

            virtual void useViewHint (const std::string& hint);
            ///< Default-implementation: ignored.

        protected:
        const CSMDoc::Document& mDocument; //for checking if drop comes from same document

        private:

            void dragEnterEvent(QDragEnterEvent *event);

            void dragMoveEvent(QDragMoveEvent *event);

        private slots:

            void selectNavigationMode (const std::string& mode);

        signals:

            void closeRequest();
    };
}

#endif