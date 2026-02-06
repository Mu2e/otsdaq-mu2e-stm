import os
import xml.etree.ElementTree as ET
import sys

# Global constant
STM_XML_ENV_VAR = "STM_XML"

# Module-level root node — parsed once at import
try:
    xml_path = os.environ.get(STM_XML_ENV_VAR)
    if not xml_path or not os.path.isfile(xml_path):
        raise FileNotFoundError(f"Environment variable '{STM_XML_ENV_VAR}' is not set or file does not exist.")
    
    tree = ET.parse(xml_path)
    root = tree.getroot()

except Exception as e:
    print(f"Critical error while loading XML config: {e}")
    sys.exit(1)

def get_xml_node_value(node_path):
    """
    Extracts the text value of a specific XML node from the preloaded XML tree.

    Args:
        node_path (str): XPath-like path to the node (e.g., "stm/starting_core_id").

    Returns:
        str: The text content of the node.

    Raises:
        ValueError: If the node is missing or empty.
    """
    node = root.find(node_path)
    if node is None or node.text is None:
        raise ValueError(f"Node <{node_path}> not found or empty in XML config.")
    
    return node.text.strip()
