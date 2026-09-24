import os
import sys
import xml.etree.ElementTree as ET

STM_XML_ENV_VAR = "STM_XML"

try:
    xml_path = os.environ.get(STM_XML_ENV_VAR)
    if not xml_path or not os.path.isfile(xml_path):
        raise FileNotFoundError(f"Environment variable '{STM_XML_ENV_VAR}' is not set or file does not exist.")

    tree = ET.parse(xml_path)
    root = tree.getroot()

    base_dir = os.path.dirname(xml_path)

    for include_tag in root.findall('include'):
        filename = include_tag.get('file')
        if filename:
            inc_path = os.path.join(base_dir, filename)
            if os.path.exists(inc_path):
                # Parse the sub-file
                inc_tree = ET.parse(inc_path)
                inc_root = inc_tree.getroot()

                # Add the block
                root.append(inc_root)

                # Remove the original <include/> tag so 
                root.remove(include_tag)

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
