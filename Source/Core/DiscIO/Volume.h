// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Common/CommonTypes.h"
#include "Common/StringUtil.h"
#include "Common/Swap.h"
#include "Core/IOS/ES/Formats.h"
#include "DiscIO/Enums.h"

namespace DiscIO
{
enum class BlobType;

struct Partition final
{
  Partition() : offset(std::numeric_limits<u64>::max()) {}
  explicit Partition(u64 offset_) : offset(offset_) {}
  bool operator==(const Partition& other) const { return offset == other.offset; }
  bool operator!=(const Partition& other) const { return !(*this == other); }
  bool operator<(const Partition& other) const { return offset < other.offset; }
  bool operator>(const Partition& other) const { return other < *this; }
  bool operator<=(const Partition& other) const { return !(*this < other); }
  bool operator>=(const Partition& other) const { return !(*this > other); }
  u64 offset;
};

const Partition PARTITION_NONE(std::numeric_limits<u64>::max() - 1);

class IVolume
{
public:
  IVolume() {}
  virtual ~IVolume() {}
  virtual bool Read(u64 _Offset, u64 _Length, u8* _pBuffer, const Partition& partition) const = 0;
  template <typename T>
  bool ReadSwapped(u64 offset, T* buffer, const Partition& partition) const
  {
    T temp;
    if (!Read(offset, sizeof(T), reinterpret_cast<u8*>(&temp), partition))
      return false;
    *buffer = Common::FromBigEndian(temp);
    return true;
  }
  virtual std::vector<Partition> GetPartitions() const { return {{}}; }
  virtual Partition GetGamePartition() const { return PARTITION_NONE; }
  bool GetTitleID(u64* buffer) const { return GetTitleID(buffer, GetGamePartition()); }
  virtual bool GetTitleID(u64* buffer, const Partition& partition) const { return false; }
  virtual const IOS::ES::TicketReader& GetTicket(const Partition& partition) const
  {
    return INVALID_TICKET;
  }
  virtual const IOS::ES::TMDReader& GetTMD(const Partition& partition) const { return INVALID_TMD; }
  std::string GetGameID() const { return GetGameID(GetGamePartition()); }
  virtual std::string GetGameID(const Partition& partition) const = 0;
  std::string GetMakerID() const { return GetMakerID(GetGamePartition()); }
  virtual std::string GetMakerID(const Partition& partition) const = 0;
  u16 GetRevision() const { return GetRevision(GetGamePartition()); }
  virtual u16 GetRevision(const Partition& partition) const = 0;
  std::string GetInternalName() const { return GetInternalName(GetGamePartition()); }
  virtual std::string GetInternalName(const Partition& partition) const = 0;
  virtual std::map<Language, std::string> GetShortNames() const { return {{}}; }
  virtual std::map<Language, std::string> GetLongNames() const { return {{}}; }
  virtual std::map<Language, std::string> GetShortMakers() const { return {{}}; }
  virtual std::map<Language, std::string> GetLongMakers() const { return {{}}; }
  virtual std::map<Language, std::string> GetDescriptions() const { return {{}}; }
  virtual std::vector<u32> GetBanner(int* width, int* height) const = 0;
  u64 GetFSTSize() const { return GetFSTSize(GetGamePartition()); }
  virtual u64 GetFSTSize(const Partition& partition) const = 0;
  std::string GetApploaderDate() const { return GetApploaderDate(GetGamePartition()); }
  virtual std::string GetApploaderDate(const Partition& partition) const = 0;
  // 0 is the first disc, 1 is the second disc
  u8 GetDiscNumber() const { return GetDiscNumber(GetGamePartition()); }
  virtual u8 GetDiscNumber(const Partition& partition) const { return 0; }
  virtual Platform GetVolumeType() const = 0;
  virtual bool SupportsIntegrityCheck() const { return false; }
  virtual bool CheckIntegrity(const Partition& partition) const { return false; }
  virtual Region GetRegion() const = 0;
  Country GetCountry() const { return GetCountry(GetGamePartition()); }
  virtual Country GetCountry(const Partition& partition) const = 0;
  virtual BlobType GetBlobType() const = 0;
  // Size of virtual disc (may be inaccurate depending on the blob type)
  virtual u64 GetSize() const = 0;
  // Size on disc (compressed size)
  virtual u64 GetRawSize() const = 0;

  static std::vector<u32> GetWiiBanner(int* width, int* height, u64 title_id);

protected:
  template <u32 N>
  std::string DecodeString(const char (&data)[N]) const
  {
    // strnlen to trim NULLs
    std::string string(data, strnlen(data, sizeof(data)));

    if (GetRegion() == Region::NTSC_J)
      return SHIFTJISToUTF8(string);
    else
      return CP1252ToUTF8(string);
  }

  static std::map<Language, std::string> ReadWiiNames(const std::vector<u8>& data);

  static const size_t NUMBER_OF_LANGUAGES = 10;
  static const size_t NAME_STRING_LENGTH = 42;
  static const size_t NAME_BYTES_LENGTH = NAME_STRING_LENGTH * sizeof(u16);
  static const size_t NAMES_TOTAL_BYTES = NAME_BYTES_LENGTH * NUMBER_OF_LANGUAGES;

  static const IOS::ES::TicketReader INVALID_TICKET;
  static const IOS::ES::TMDReader INVALID_TMD;
};

std::unique_ptr<IVolume> CreateVolumeFromFilename(const std::string& filename);
std::unique_ptr<IVolume> CreateVolumeFromDirectory(const std::string& directory, bool is_wii,
                                                   const std::string& apploader = "",
                                                   const std::string& dol = "");

}  // namespace
