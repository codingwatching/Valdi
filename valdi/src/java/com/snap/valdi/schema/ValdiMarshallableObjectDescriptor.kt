package com.snap.valdi.schema

import androidx.annotation.Keep
import com.snap.valdi.exceptions.ValdiFatalException
import com.snap.valdi.utils.arrayMap
import java.io.ByteArrayOutputStream
import java.lang.Exception
import java.lang.reflect.Method
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.charset.StandardCharsets
import kotlin.reflect.KClass

@Keep
class ValdiMarshallableObjectDescriptor

private constructor(val type: Int,
                    val schema: String,
                    val proxyClass: Class<*>?,
                    val typeReferences: Array<String>?,
                    val propertyReplacements: String?) {

    companion object {

        const val TYPE_CLASS = 1
        const val TYPE_INTERFACE = 2
        const val TYPE_STRING_ENUM = 3
        const val TYPE_INT_ENUM = 4
        const val TYPE_UNTYPED = 5

        private val objectImplementsMethodCache = hashMapOf<Class<*>, MutableMap<Method, Boolean>>()

        @JvmStatic
        private fun make(type: Int,
                         schema: String,
                         proxyClass: Class<*>?,
                         typeReferences: Array<KClass<*>>,
                         propertyReplacements: String): ValdiMarshallableObjectDescriptor {
            val typeReferenceNames = typeReferences.takeIf { it.isNotEmpty() }?.arrayMap { it.java.name }
            val propertyReplacementsStr = propertyReplacements.takeIf { it.isNotEmpty() }
            return ValdiMarshallableObjectDescriptor(
                    type,
                    schema,
                    proxyClass,
                    typeReferenceNames,
                    propertyReplacementsStr)
        }

        @JvmStatic
        private fun forClass(schema: String,
                             typeReferences: Array<KClass<*>>,
                             propertyReplacements: String): ValdiMarshallableObjectDescriptor {
            return make(
                    TYPE_CLASS,
                    schema,
                    null,
                    typeReferences,
                    propertyReplacements)
        }

        @JvmStatic
        private fun forInterface(schema: String,
                                 proxyClass: Class<*>,
                                 typeReferences: Array<KClass<*>>,
                                 propertyReplacements: String): ValdiMarshallableObjectDescriptor {
            return make(
                    TYPE_INTERFACE,
                    schema,
                    proxyClass,
                    typeReferences,
                    propertyReplacements)
        }

        @JvmStatic
        private fun forStringEnum(schema: String,
                                  propertyReplacements: String): ValdiMarshallableObjectDescriptor {
            return make(
                    TYPE_STRING_ENUM,
                    schema,
                    null,
                    emptyArray(),
                    propertyReplacements)
        }

        @JvmStatic
        private fun forIntEnum(schema: String,
                               propertyReplacements: String): ValdiMarshallableObjectDescriptor {
            return make(
                    TYPE_INT_ENUM,
                    schema,
                    null,
                    emptyArray(),
                    propertyReplacements)
        }

        @JvmStatic
        private fun forUntyped(): ValdiMarshallableObjectDescriptor {
            return make(TYPE_UNTYPED, "u", null, emptyArray(), "")
        }

        @JvmStatic
        private fun forFunction(schema: String,
                                typeReferences: Array<KClass<*>>,
                                propertyReplacements: String): ValdiMarshallableObjectDescriptor {
            val resolvedPropertyReplacements = if (propertyReplacements.isNotEmpty()) propertyReplacements else "0:'_invoker'"
            return forClass(schema, typeReferences, resolvedPropertyReplacements)
        }

        @Keep
        @JvmStatic
        fun getDescriptorForClass(cls: Class<*>): ValdiMarshallableObjectDescriptor {
            try {
                return descriptorFromAnnotations(cls)
            } catch (exc: Throwable) {
                ValdiFatalException.handleFatal(exc, "Could not resolve descriptor for class ${cls.name}")
            }
        }

        @JvmStatic
        private fun descriptorFromAnnotations(cls: Class<*>): ValdiMarshallableObjectDescriptor {
            if (cls.isInterface) {
                val valdiInterface = cls.getAnnotation(ValdiInterface::class.java)
                if (valdiInterface != null) {
                    return forInterface(
                            valdiInterface.schema,
                            valdiInterface.proxyClass.java,
                            valdiInterface.typeReferences,
                            valdiInterface.propertyReplacements)
                }

                val valdiUntyped = cls.getAnnotation(ValdiUntypedClass::class.java)
                if (valdiUntyped != null) {
                    return forUntyped()
                }
            }

            if (cls.isEnum) {
                val valdiEnum = cls.getAnnotation(ValdiEnum::class.java)
                if (valdiEnum != null) {
                    return when (valdiEnum.type) {
                        ValdiEnumType.INT -> forIntEnum(valdiEnum.schema, valdiEnum.propertyReplacements)
                        ValdiEnumType.STRING -> forStringEnum(valdiEnum.schema, valdiEnum.propertyReplacements)
                    }
                }
            }

            val valdiClass = cls.getAnnotation(ValdiClass::class.java)
            if (valdiClass != null) {
                return forClass(
                        valdiClass.schema,
                        valdiClass.typeReferences,
                        valdiClass.propertyReplacements)
            }

            val valdiFunction = cls.getAnnotation(ValdiFunctionClass::class.java)
            if (valdiFunction != null) {
                return forFunction(
                        valdiFunction.schema,
                        valdiFunction.typeReferences,
                        valdiFunction.propertyReplacements)
            }

            throw Exception("Could not resolve Valdi Annotation")
        }

        // --- Batched descriptor fetch (JNI-overhead reduction). See valdi-jni-batch-plan.md ---

        /** Binary format version of the buffer produced by [getDescriptorClosure]. Must match the C++
         *  parser in AndroidValueMarshallerRegistry. Bump on any layout change. */
        const val DESCRIPTOR_CLOSURE_FORMAT_VERSION: Int = 2

        /**
         * A class's Valdi type references paired with the indices into that array that are lazily-
         * resolved function-return closures. Both are read from the single Valdi annotation, so the
         * walk does one getAnnotation sweep per class instead of two. References are already-resolved
         * [KClass] (not names), so traversal is classloader-safe. Enums and untyped types have none.
         */
        private class ClassReferences(val references: Array<KClass<*>>, val lazyReturnIndices: IntArray)

        private val EMPTY_CLASS_REFERENCES = ClassReferences(emptyArray(), IntArray(0))

        @JvmStatic
        private fun classReferences(cls: Class<*>): ClassReferences {
            cls.getAnnotation(ValdiClass::class.java)?.let {
                return ClassReferences(it.typeReferences, it.lazyReturnTypeReferences)
            }
            cls.getAnnotation(ValdiInterface::class.java)?.let {
                return ClassReferences(it.typeReferences, it.lazyReturnTypeReferences)
            }
            cls.getAnnotation(ValdiFunctionClass::class.java)?.let {
                return ClassReferences(it.typeReferences, it.lazyReturnTypeReferences)
            }
            return EMPTY_CLASS_REFERENCES
        }

        /**
         * Whether [cls] is a Valdi marshallable type (carries one of the Valdi schema annotations).
         * Type references can point at non-marshallable classes (e.g. java.lang.Object from an untyped
         * field); those must be skipped by the closure walk — [getDescriptorForClass] would fatal on
         * them, and the registry resolves them through its own path instead.
         */
        @JvmStatic
        private fun hasValdiDescriptor(cls: Class<*>): Boolean {
            return cls.isAnnotationPresent(ValdiClass::class.java) ||
                cls.isAnnotationPresent(ValdiInterface::class.java) ||
                cls.isAnnotationPresent(ValdiFunctionClass::class.java) ||
                cls.isAnnotationPresent(ValdiEnum::class.java) ||
                cls.isAnnotationPresent(ValdiUntypedClass::class.java)
        }

        private fun writeLengthPrefixed(out: ByteArrayOutputStream, value: String) {
            // Payloads are ASCII (schema syntax + JVM class names), so ISO-8859-1 is a 1:1 byte copy.
            // Length is u32: a large @ExportModel's schema can exceed 64KB, and a u16 prefix would
            // silently truncate it (size and 0xFFFF) and desync the rest of the buffer.
            val bytes = value.toByteArray(StandardCharsets.ISO_8859_1)
            out.write(bytes.size and 0xFF)
            out.write((bytes.size ushr 8) and 0xFF)
            out.write((bytes.size ushr 16) and 0xFF)
            out.write((bytes.size ushr 24) and 0xFF)
            out.write(bytes)
        }

        /**
         * JVM class names already visited by a prior [getDescriptorClosure] call (every class packed
         * into a buffer, plus non-marshallable references that were skipped). Accumulated across calls
         * and reused as the walk's `visited` set so each call prunes to the not-yet-resolved frontier,
         * skipping already-cached classes and their subtrees.
         *
         * This mirrors the C++ registry's descriptor cache without C++ echoing it back each call: that
         * cache belongs to a process-singleton ([ValdiValueMarshallerRegistry.shared]), is append-only,
         * and is never cleared, so the two stay in sync by construction. Every call originates from the
         * C++ registry under its schema-registry lock, so access here is already serialized — no extra
         * synchronization needed. (If the two ever diverged, e.g. C++ dropped a buffer, the registry
         * self-heals via its legacy per-class fallback.)
         */
        private val resolvedClassNames = HashSet<String>()

        /**
         * Resolve [root] and its transitive type-reference closure, returning every not-yet-resolved
         * descriptor packed into one direct [ByteBuffer] for zero-copy parsing in C++. Replaces N
         * per-class getDescriptorForClass JNI round-trips (+ per-field/array read-backs) with a single
         * batched fetch; C++ caches the parsed descriptors by name so reference recursion becomes cache
         * hits. Classes packed by a prior call are tracked in [resolvedClassNames] and skipped (with
         * their subtrees), so each call costs only the new frontier, not the whole accumulated cache.
         *
         * Layout (little-endian, ASCII payloads):
         *   [u8 version][u32 count] then, per entry:
         *   [u32+className][u8 type][u32+schema][u32+propertyReplacements][u32+proxyClassName]
         *   [u16 refCount]{ [u32+refName] }
         *
         * [skipLazyReturnTypeReferences] is passed by the native caller from the single authoritative
         * lazy-resolution flag (see AndroidValueMarshallerRegistry / lazyFunctionReturnMarshallerFlag),
         * so there is no separate Kotlin-side toggle to keep in sync. When true, the walk does not recurse
         * into references reachable only through lazily-resolved function sync-value returns, keeping their
         * descriptors off the startup critical path (the registry resolves them on demand via its legacy
         * per-class path); a mismatch only over/under-fetches (self-healing), never breaks correctness.
         */
        @Keep
        @JvmStatic
        fun getDescriptorClosure(root: Class<*>, skipLazyReturnTypeReferences: Boolean): ByteBuffer {
            // Reuse the running set directly as `visited`: classes resolved by earlier calls (and their
            // subtrees) are skipped, so the walk only touches the new frontier. Serialized by the C++
            // registry lock (see [resolvedClassNames]).
            val visited = resolvedClassNames
            val queue = ArrayDeque<Class<*>>()
            queue.addLast(root)

            val body = ByteArrayOutputStream(8192)
            var count = 0

            while (queue.isNotEmpty()) {
                // Use removeAt(0): the remove-first extension resolves to the Java 21
                // SequencedCollection method and NoSuchMethodErrors on older Android runtimes
                // (banned by check_unsupported_android_15_method_usage).
                val cls = queue.removeAt(0)
                if (!visited.add(cls.name)) {
                    continue
                }
                if (!hasValdiDescriptor(cls)) {
                    // Non-marshallable reference (e.g. java.lang.Object from an untyped field). Don't
                    // fetch (getDescriptorForClass would fatal) or recurse; the registry resolves such
                    // references via its own path, falling back to legacy per-class resolution.
                    continue
                }

                val descriptor = getDescriptorForClass(cls)
                writeLengthPrefixed(body, cls.name)
                body.write(descriptor.type and 0xFF)
                writeLengthPrefixed(body, descriptor.schema)
                writeLengthPrefixed(body, descriptor.propertyReplacements ?: "")
                writeLengthPrefixed(body, descriptor.proxyClass?.name ?: "")

                val refNames = descriptor.typeReferences ?: emptyArray()
                body.write(refNames.size and 0xFF)
                body.write((refNames.size ushr 8) and 0xFF)
                for (refName in refNames) {
                    writeLengthPrefixed(body, refName)
                }
                count++

                val classRefs = classReferences(cls)
                val refs = classRefs.references
                // Skip references reachable only through lazily-resolved function returns: leaving them
                // unwalked keeps their descriptors out of the buffer, so the registry resolves them on
                // demand (off the critical path) instead of eagerly here. Packed refNames above still list
                // them, so an eager reference from any other class re-adds them to the frontier.
                val lazyRefs = if (skipLazyReturnTypeReferences) classRefs.lazyReturnIndices else IntArray(0)
                for (i in refs.indices) {
                    if (i in lazyRefs) {
                        continue
                    }
                    queue.addLast(refs[i].java)
                }
            }

            val bodyBytes = body.toByteArray()
            val buffer = ByteBuffer.allocateDirect(1 + 4 + bodyBytes.size).order(ByteOrder.LITTLE_ENDIAN)
            buffer.put(DESCRIPTOR_CLOSURE_FORMAT_VERSION.toByte())
            buffer.putInt(count)
            buffer.put(bodyBytes)
            buffer.flip()
            return buffer
        }

        @JvmStatic
        private fun resolveClassImplementsMethod(cls: Class<*>, method: Method): Boolean {
            return try {
                val methodInClass = cls.getMethod(method.name, *method.parameterTypes)
                methodInClass.getAnnotation(ValdiOptionalMethod::class.java) == null
            } catch (exc: NoSuchMethodError) {
                false
            }
        }

        @Keep
        @JvmStatic
        fun objectImplementsMethod(obj: Any, method: Method): Boolean {
            val cls = obj.javaClass

            return synchronized(objectImplementsMethodCache) {
                var methodCache = objectImplementsMethodCache[cls]
                if (methodCache == null) {
                    methodCache = hashMapOf()
                    objectImplementsMethodCache[cls] = methodCache
                }

                var implements = methodCache[method]
                if (implements == null) {
                    implements = resolveClassImplementsMethod(cls, method)
                    methodCache[method] = implements
                }

                implements
            }
        }
    }

}