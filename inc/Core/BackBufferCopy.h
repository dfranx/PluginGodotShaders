#pragma once
#include <Core/PipelineItem.h>

namespace gd
{
	namespace pipe
	{
		class BackBufferCopy : public PipelineItem
		{
		public:
			BackBufferCopy() { Type = PipelineItemType::BackBufferCopy; }
		};
	}
}