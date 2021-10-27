#pragma once

#include "../Container/Container.h"
#include "../Container/ContainerView.h"
#include "NamespaceDef.h"

namespace mfuse
{
	class NamespaceDef;

	enum class namespaceFilterMode_e
	{
		/** No namespace filtering. */
		None,
		/** Only includes specified namespaces. */
		Inclusive,
		/** Only excludes specified namespaces. */
		Exclusive
	};

	class NamespaceManager
	{
	public:
		NamespaceManager();
		~NamespaceManager();

		/** Return the namespace filter mode. */
		mfuse_EXPORTS namespaceFilterMode_e GetFilterMode() const;

		/** Set the namespace filter mode.*/
		mfuse_EXPORTS void SetFilterMode(namespaceFilterMode_e newMode);

		/** Return whether or not the namespace is allowed to be used. */
		mfuse_EXPORTS bool IsNamespaceAllowed(const NamespaceDef* def) const;
		/** Return whether or not the object connected to a namespace is allowed. */
		mfuse_EXPORTS bool IsObjectInNamespaceAllowed(const ObjectInNamespace& obj) const;

		/**
		 * Set the list of namespaces to be filtered.
		 *
		 * @param newDefList List of namespaces.
		 */
		mfuse_EXPORTS void SetFilteredNamespace(const con::ContainerView<const NamespaceDef*>& newDefList);

	private:
		con::Container<namespaceNum_t> defList;
		namespaceFilterMode_e filterMode;
	};
}
