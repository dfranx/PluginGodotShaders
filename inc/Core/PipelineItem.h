#pragma once
#include <Core/Settings.h>
#include <PluginAPI/Plugin.h>
#include <vector>

namespace gd
{
	enum class PipelineItemType
	{
		CanvasMaterial,
		Sprite,
		BackBufferCopy
	};

	class PipelineItem
	{
	public:
		virtual ~PipelineItem() {};

		// local copy of the data stored in SHADERed:
		char Name[PIPELINE_ITEM_NAME_LENGTH];
		std::vector<PipelineItem*> Items;
		ed::IPlugin1* Owner;
		PipelineItemType Type;
	};
}