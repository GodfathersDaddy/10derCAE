#pragma once

#include "AppTypes.h"

class WorkspaceCatalog {
public:
    static const QVector<WorkspaceDefinition>& all();
    static const WorkspaceDefinition& get(WorkspaceId id);
};

