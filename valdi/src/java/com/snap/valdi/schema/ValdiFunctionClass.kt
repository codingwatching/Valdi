package com.snap.valdi.schema

import kotlin.reflect.KClass

/**
 * Annotation added to every types generated from TypeScript's
 * @GenerateNativeFunction annotation.
 */
@Target(AnnotationTarget.CLASS)
@Retention(AnnotationRetention.RUNTIME)
annotation class ValdiFunctionClass(
        val schema: String,
        /**
         * An array containing the foreign type references used in the schema.
         * The final schema will be computed using the class names from this type references.
         */
        val typeReferences: Array<KClass<*>> = [],
        val propertyReplacements: String = "",
        /**
         * Indices into [typeReferences] whose only use in the schema is as a lazily-resolved function
         * sync-value return type (mirrors the native lazy function-return marshaller deferral). When
         * lazy resolution is enabled, the batched descriptor-closure walk skips recursing into these so
         * their descriptors stay off the startup critical path; the registry resolves them on demand.
         */
        val lazyReturnTypeReferences: IntArray = [],
)