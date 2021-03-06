#include "PathTree.h"
#include <QDebug>
#include <QUrl>

PathTree::PathTree(QObject *parent)
    :QObject(parent),
      m_root(new PathTreeNode("")),
      m_emptyFunc()
{
}

bool PathTree::registerAPath(const QString &path, const std::function<void(HttpRequest &, HttpResponse &)> &in,enum PathTreeNode::HttpVerb verb)
{
    /*qDebug()<<"Register a Handler:";
    qDebug()<<"Path:"<<path;
    qDebug()<<"Object:"<<object;
    qDebug()<<"Method Name:"<<methodName;
    //qDebug()<<"Method Count:"<<((Zoidblog*)object)->metaObject()->methodCount();
    //qDebug()<<"Method Signature:"<<((Zoidblog*)object)->metaObject()->method(4).signature();
    qDebug()<<"Method ID:"<<object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodName.toStdString().c_str()));
    */

    if(!path.isEmpty() && in)
    {
        if(path.at(0)!='/')
        {
            return false;
        }
        else if(path.length()==1)
        {
            if(verb==PathTreeNode::GET)
            {
                return m_root->setGetHandler(in);
            }
            else
            {
                return m_root->setPostHandler(in);
            }
        }
        else
        {
            int posBegin=1;
            int posEnd=1;

            QWeakPointer<PathTreeNode> currentPathTreeNode = m_root.toWeakRef();

            for(posEnd = 1; posEnd < path.length(); ++posEnd)
            {
                if(path.at(posEnd)=='/' || posEnd == path.length() - 1)
                {
                    if(posEnd == path.count() - 1)
                        posEnd++;

                    QString pathName = path.mid(posBegin,posEnd-posBegin);

                    if (pathName == "." || pathName == "..")
                    {
                        return false;
                    }

                    if(!currentPathTreeNode.data()->hasChild(pathName))
                    {
                        currentPathTreeNode.data()->addChild(pathName);
                    }

                    currentPathTreeNode=currentPathTreeNode.data()->getChild(pathName);

                    posEnd=posBegin=posEnd+1;
                }
            }

            if(verb == PathTreeNode::GET)
            {
                return currentPathTreeNode.data()->setGetHandler(in);
            }
            else
            {
                return currentPathTreeNode.data()->setPostHandler(in);
            }
        }
    }
    else
    {
        return false;
    }
}

const std::function<void(HttpRequest &, HttpResponse &)> & PathTree::getTaskHandlerByPath(const QString &path,enum PathTreeNode::HttpVerb type)
{
    if(!path.isEmpty() && path.at(0)=='/')
    {
        if(path.length()==1)
        {
            if(type==PathTreeNode::GET)
            {
                return m_root->getHandler();
            }
            else
            {
                return m_root->postHandler();
            }
        }
        else
        {
            int posBegin=1;
            int posEnd=1;

            QWeakPointer<PathTreeNode> currentPathTreeNode = m_root.toWeakRef();

            for(posEnd=1;posEnd<path.count();++posEnd)
            {
                if(path.at(posEnd)=='/' || posEnd==path.count()-1)
                {
                    if(posEnd==path.count()-1)
                        posEnd++;

                    QString pathName=path.mid(posBegin,posEnd-posBegin);

                    if (pathName == "." || pathName == "..")
                    {
                        return m_emptyFunc;
                    }

                    if(currentPathTreeNode.data()->hasChild(pathName))
                    {
                        currentPathTreeNode=currentPathTreeNode.data()->getChild(pathName);
                        posBegin = posEnd = posEnd + 1;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if(type==PathTreeNode::GET)
            {
                return currentPathTreeNode.data()->getHandler();
            }
            else
            {
                return currentPathTreeNode.data()->postHandler();
            }
        }
    }
    else
    {
        return m_emptyFunc;
    }
}
