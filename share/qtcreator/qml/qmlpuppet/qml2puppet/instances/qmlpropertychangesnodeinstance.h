/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/
#ifndef QMLPROPERTYCHANGESNODEINSTANCE_H
#define QMLPROPERTYCHANGESNODEINSTANCE_H

#include "objectnodeinstance.h"

#include <QPair>
#include <QWeakPointer>

namespace QmlDesigner {

namespace Internal {

class QmlPropertyChangesNodeInstance;

class QmlPropertyChangesNodeInstance : public ObjectNodeInstance
{
public:
    typedef QSharedPointer<QmlPropertyChangesNodeInstance> Pointer;
    typedef QWeakPointer<QmlPropertyChangesNodeInstance> WeakPointer;

    static Pointer create(QObject *objectToBeWrapped);

    virtual void setPropertyVariant(const PropertyName &name, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void setPropertyBinding(const PropertyName &name, const QString &expression) Q_DECL_OVERRIDE;
    virtual QVariant property(const PropertyName &name) const Q_DECL_OVERRIDE;
    virtual void resetProperty(const PropertyName &name) Q_DECL_OVERRIDE;

    void reparent(const ObjectNodeInstance::Pointer &oldParentInstance, const PropertyName &oldParentProperty, const ObjectNodeInstance::Pointer &newParentInstance, const PropertyName &newParentProperty) Q_DECL_OVERRIDE;

protected:
    QmlPropertyChangesNodeInstance(QObject *object);
};

} // namespace Internal
} // namespace QmlDesigner

//QML_DECLARE_TYPE(QmlDesigner::Internal::QmlPropertyChangesObject)

#endif // QMLPROPERTYCHANGESNODEINSTANCE_H
