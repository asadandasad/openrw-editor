#ifndef ASSET_BROWSER_H
#define ASSET_BROWSER_H

#include "types.h"
#include <QWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QFileSystemWatcher>
#include <QMimeData>
#include <QDrag>

// Asset browser widget for browsing and dragging game assets
class AssetBrowser : public QWidget {
    Q_OBJECT
    
public:
    enum AssetType {
        All,
        Models,      // DFF files
        Textures,    // TXD files
        Scripts,     // Lua/AngelScript files
        Audio,       // Audio files
        Data         // IDE/IPL/DAT files
    };
    
    struct AssetInfo {
        QString name;
        QString path;
        QString relativePath;
        AssetType type;
        qint64 size;
        QDateTime modified;
        QString description;
        QPixmap thumbnail;
    };
    
    explicit AssetBrowser(QWidget* parent = nullptr);
    
    void setRootPath(const QString& path);
    QString getRootPath() const;
    
    void setAssetFilter(AssetType type);
    AssetType getAssetFilter() const;
    
    void refreshAssets();
    QVector<AssetInfo> getSelectedAssets() const;
    
signals:
    void assetSelected(const AssetInfo& asset);
    void assetDoubleClicked(const AssetInfo& asset);
    void assetDragStarted(const AssetInfo& asset);
    
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
private slots:
    void onDirectoryChanged(const QString& path);
    void onFileChanged(const QString& path);
    void onFolderSelectionChanged();
    void onAssetSelectionChanged();
    void onAssetDoubleClicked(QListWidgetItem* item);
    void onSearchTextChanged(const QString& text);
    void onFilterChanged(int index);
    void onRefreshClicked();
    
private:
    // UI setup
    void setupUI();
    void setupToolbar();
    void setupFolderTree();
    void setupAssetList();
    
    // Asset management
    void scanDirectory(const QString& path);
    void loadAssetInfo(const QString& filePath);
    AssetType getAssetTypeFromFile(const QString& filePath) const;
    QString getAssetDescription(const AssetInfo& asset) const;
    QPixmap generateThumbnail(const AssetInfo& asset) const;
    
    // Filtering and searching
    void applyFilters();
    bool matchesFilter(const AssetInfo& asset) const;
    bool matchesSearch(const AssetInfo& asset, const QString& searchText) const;
    
    // Drag and drop
    void startDrag(const AssetInfo& asset);
    QMimeData* createMimeData(const AssetInfo& asset) const;
    
    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    QSplitter* m_splitter;
    
    // Toolbar
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QPushButton* m_refreshButton;
    QLabel* m_statusLabel;
    
    // Folder tree
    QTreeWidget* m_folderTree;
    
    // Asset list
    QListWidget* m_assetList;
    
    // Data
    QString m_rootPath;
    AssetType m_currentFilter;
    QVector<AssetInfo> m_allAssets;
    QVector<AssetInfo> m_filteredAssets;
    QFileSystemWatcher* m_fileWatcher;
    
    // Asset type mappings
    QMap<QString, AssetType> m_extensionMap;
    QMap<AssetType, QString> m_typeNames;
    QMap<AssetType, QStringList> m_typeExtensions;
};

// Custom list widget item for assets
class AssetListItem : public QListWidgetItem {
public:
    AssetListItem(const AssetBrowser::AssetInfo& asset, QListWidget* parent = nullptr);
    
    const AssetBrowser::AssetInfo& getAssetInfo() const;
    
protected:
    void setData(int role, const QVariant& value) override;
    QVariant data(int role) const override;
    
private:
    AssetBrowser::AssetInfo m_assetInfo;
};

// Custom list widget with drag support
class AssetListWidget : public QListWidget {
    Q_OBJECT
    
public:
    explicit AssetListWidget(QWidget* parent = nullptr);
    
signals:
    void assetDragStarted(const AssetBrowser::AssetInfo& asset);
    
protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    
private:
    QPoint m_dragStartPosition;
    bool m_dragEnabled;
};

#endif // ASSET_BROWSER_H

