// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Common/CommonTypes.h"
#include "DiscIO/Volume.h"

namespace DiscIO
{
// file info of an FST entry
struct SFileInfo
{
  u64 m_NameOffset = 0u;
  u64 m_Offset = 0u;
  u64 m_FileSize = 0u;
  std::string m_FullPath;

  bool IsDirectory() const { return (m_NameOffset & 0xFF000000) != 0; }
  SFileInfo(u64 name_offset, u64 offset, u64 filesize)
      : m_NameOffset(name_offset), m_Offset(offset), m_FileSize(filesize)
  {
  }

  SFileInfo(SFileInfo const&) = default;
  SFileInfo() = default;
};

class IFileSystem
{
public:
  IFileSystem(const IVolume* _rVolume, const Partition& partition);

  virtual ~IFileSystem();
  virtual bool IsValid() const = 0;
  virtual const std::vector<SFileInfo>& GetFileList() = 0;
  virtual u64 GetFileSize(const std::string& _rFullPath) = 0;
  virtual u64 ReadFile(const std::string& _rFullPath, u8* _pBuffer, u64 _MaxBufferSize,
                       u64 _OffsetInFile = 0) = 0;
  virtual bool ExportFile(const std::string& _rFullPath, const std::string& _rExportFilename) = 0;
  virtual bool ExportApploader(const std::string& _rExportFolder) const = 0;
  virtual bool ExportDOL(const std::string& _rExportFolder) const = 0;
  virtual std::string GetFileName(u64 _Address) = 0;
  virtual std::optional<u64> GetBootDOLOffset() const = 0;
  virtual std::optional<u32> GetBootDOLSize(u64 dol_offset) const = 0;

  virtual const Partition GetPartition() const { return m_partition; }
protected:
  const IVolume* const m_rVolume;
  const Partition m_partition;
};

std::unique_ptr<IFileSystem> CreateFileSystem(const IVolume* volume, const Partition& partition);

}  // namespace
