#pragma once
#include <memory>
namespace fb {
	// T assumed to be the class T of shared_ptr
	template <class T>
	class QuadNode
	{
	protected:
		using TPtr = std::shared_ptr<T>;
		using TWeakPtr = std::weak_ptr<T>;
		TPtr mQuadTreeChild[4];
		TWeakPtr mQuadTreeParent;

	public:
		bool HasChildrenNodes() const {
			return mQuadTreeChild[0] != nullptr;
		}
		bool HasParentNode() const {
			return !mQuadTreeParent.expired()
		}

		TPtr GetParent() const {
			return mQuadTreeParent;
		}

		void SetChildrenNodes(const std::vector<TPtr>& children) {
			if (children.size() != 4) {
				assert(0 && "Invalid children size");
				return;
			}
			for (int i = 0; i < 4; ++i) {
				mQuadTreeChild[i] = children[i];				
			}
		}

		void SetParentNode(TPtr parent) {
			mQuadTreeParent = parent;
		}

		std::vector<TPtr> GetChildrenNodes() const {
			return{ mQuadTreeChild[0], mQuadTreeChild[1] , mQuadTreeChild[2] , mQuadTreeChild[3] };
		}
	};
}
