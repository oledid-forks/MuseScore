/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_PROJECT_PROJECTCONFIGURATION_H
#define MU_PROJECT_PROJECTCONFIGURATION_H

#include "types/val.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"
#include "accessibility/iaccessibilityconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "cloud/icloudconfiguration.h"
#include "languages/ilanguagesservice.h"

#include "../iprojectconfiguration.h"

namespace mu::project {
class ProjectConfiguration : public IProjectConfiguration
{
    INJECT(IGlobalConfiguration, globalConfiguration)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(muse::cloud::ICloudConfiguration, cloudConfiguration)
    INJECT(muse::accessibility::IAccessibilityConfiguration, accessibilityConfiguration)
    INJECT(io::IFileSystem, fileSystem)
    INJECT(languages::ILanguagesService, languagesService)

public:
    void init();

    io::path_t recentFilesJsonPath() const override;
    ByteArray compatRecentFilesData() const override;

    io::path_t myFirstProjectPath() const override;

    io::paths_t availableTemplateDirs() const override;
    io::path_t templateCategoriesJsonPath(const io::path_t& templatesDir) const override;

    io::path_t userTemplatesPath() const override;
    void setUserTemplatesPath(const io::path_t& path) override;
    async::Channel<io::path_t> userTemplatesPathChanged() const override;

    io::path_t lastOpenedProjectsPath() const override;
    void setLastOpenedProjectsPath(const io::path_t& path) override;

    io::path_t lastSavedProjectsPath() const override;
    void setLastSavedProjectsPath(const io::path_t& path) override;

    io::path_t userProjectsPath() const override;
    void setUserProjectsPath(const io::path_t& path) override;
    async::Channel<io::path_t> userProjectsPathChanged() const override;
    io::path_t defaultUserProjectsPath() const override;

    bool shouldAskSaveLocationType() const override;
    void setShouldAskSaveLocationType(bool shouldAsk) override;

    bool isCloudProject(const io::path_t& projectPath) const override;
    bool isLegacyCloudProject(const io::path_t& projectPath) const override;
    io::path_t cloudProjectPath(int scoreId) const override;
    int cloudScoreIdFromPath(const io::path_t& projectPath) const override;

    io::path_t cloudProjectSavingPath(int scoreId = 0) const override;

    io::path_t defaultSavingFilePath(INotationProjectPtr project, const std::string& filenameAddition = "",
                                     const std::string& suffix = "") const override;

    SaveLocationType lastUsedSaveLocationType() const override;
    void setLastUsedSaveLocationType(SaveLocationType type) override;

    bool shouldWarnBeforePublish() const override;
    void setShouldWarnBeforePublish(bool shouldWarn) override;

    bool shouldWarnBeforeSavingPubliclyToCloud() const override;
    void setShouldWarnBeforeSavingPubliclyToCloud(bool shouldWarn) override;

    int homeScoresPageTabIndex() const override;
    void setHomeScoresPageTabIndex(int index) override;

    HomeScoresPageViewType homeScoresPageViewType() const override;
    void setHomeScoresPageViewType(HomeScoresPageViewType type) override;

    QColor templatePreviewBackgroundColor() const override;
    async::Notification templatePreviewBackgroundChanged() const override;

    PreferredScoreCreationMode preferredScoreCreationMode() const override;
    void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) override;

    MigrationOptions migrationOptions(MigrationType type) const override;
    void setMigrationOptions(MigrationType type, const MigrationOptions& opt, bool persistent = true) override;

    bool isAutoSaveEnabled() const override;
    void setAutoSaveEnabled(bool enabled) override;
    async::Channel<bool> autoSaveEnabledChanged() const override;

    int autoSaveIntervalMinutes() const override;
    void setAutoSaveInterval(int minutes) override;
    async::Channel<int> autoSaveIntervalChanged() const override;

    bool alsoShareAudioCom() const override;
    void setAlsoShareAudioCom(bool share) override;
    async::Channel<bool> alsoShareAudioComChanged() const override;

    bool showAlsoShareAudioComDialog() const override;
    void setShowAlsoShareAudioComDialog(bool show) override;

    bool hasAskedAlsoShareAudioCom() const override;
    void setHasAskedAlsoShareAudioCom(bool has) override;

    io::path_t newProjectTemporaryPath() const override;

    bool isAccessibleEnabled() const override;

    bool shouldDestinationFolderBeOpenedOnExport() const override;
    void setShouldDestinationFolderBeOpenedOnExport(bool shouldDestinationFolderBeOpenedOnExport) override;

    QUrl supportForumUrl() const override;

    bool openDetailedProjectUploadedDialog() const override;
    void setOpenDetailedProjectUploadedDialog(bool show) override;

    bool hasAskedAudioGenerationSettings() const override;
    void setHasAskedAudioGenerationSettings(bool has) override;

    GenerateAudioTimePeriodType generateAudioTimePeriodType() const override;
    void setGenerateAudioTimePeriodType(GenerateAudioTimePeriodType type) override;
    int numberOfSavesToGenerateAudio() const override;
    void setNumberOfSavesToGenerateAudio(int number) override;

    io::path_t temporaryMp3FilePathTemplate() const override;

    io::path_t projectBackupPath(const io::path_t& projectPath) const override;

    bool showCloudIsNotAvailableWarning() const override;
    void setShowCloudIsNotAvailableWarning(bool show) override;

    bool disableVersionChecking() const override;
    void setDisableVersionChecking(bool disable) override;

private:
    io::path_t appTemplatesPath() const;
    io::path_t legacyCloudProjectsPath() const;
    io::path_t cloudProjectsPath() const;

    async::Channel<io::path_t> m_userTemplatesPathChanged;
    async::Channel<io::path_t> m_userScoresPathChanged;

    int m_homeScoresPageTabIndex = 0;
    async::Notification m_homeScoresPageTabIndexChanged;

    async::Channel<bool> m_autoSaveEnabledChanged;
    async::Channel<int> m_autoSaveIntervalChanged;

    async::Channel<bool> m_alsoShareAudioComChanged;

    mutable std::map<MigrationType, MigrationOptions> m_migrationOptions;
};
}

#endif // MU_PROJECT_PROJECTCONFIGURATION_H
